// ============================================================================
//  Guerrilla Mode NATIVE parity - garrison spawn-and-command (issue #3 item 5).
//    The retired spawning.sqs distance-cache is engine code now
//    (Game/Guerrilla/GarrisonCache.cpp). This proves, on full CWA data (Abel):
//      1. the cache AUTO-SPAWNS the Outpost garrison because the player boots
//         at the Camp, 600 m away - inside the 800 m cacheRadius;
//      2. the spawned strength matches the config reserve (garrison = 6,
//         single group under groupSize = 12);
//      3. the spawned groups are real commandable AI: doMove displaces a unit.
//
//  Mission: tests/integration/missions/guerrilla_native.abel - init.sqs and
//  scripts/ are byte-identical to the canonical Guerrilla.Demo core, so this
//  exercises the shipped mode, not a reimplementation.
//
//  gn* globals are used across statements (the harness evaluates each
//  ;/newline statement in its own context, so locals would not survive).
// ============================================================================

// -- mode fully booted (init.sqs published the helper-table sentinel) ---------
triSimUntil { GM_LIB_READY }

// -- zone table sanity: explicit zones resolve by name, seeded CITY zones
//    from Abel's Names config prove seedCities ran (14 towns, minus Houdan
//    deduped against the explicit Village zone -> 3 + 13 = 16) ---------------
gnOut = gmZoneIndex "Outpost"
triAssertGe [gnOut, 0]
triAssertGe [gmZoneCount, 4]
triAssertEq [((gmZone gnOut) select 2), gmOccupierSide]

// -- 1) NATIVE AUTO-SPAWN: no script pokes the cache; being near is enough ---
triSimUntil { gmGarrisonSpawned gnOut }

// -- 2) full configured strength, one group ----------------------------------
triSimUntil { (gmGarrisonLive gnOut) >= 6 }
triAssertEq [(gmGarrisonLive gnOut), 6]
gnGrps = gmGarrisonGroups gnOut
triAssertEq [(count gnGrps), 1]

// -- 3) COMMANDABLE: doMove the group leader ~110 m and require displacement.
//    Time-bounded (polled), not frame-bounded, so it is framerate-agnostic.
//    doStop FIRST: the garrison group holds an ACSENTRY waypoint
//    (GarrisonCache::SetHoldPosture) whose FSM re-plan races a bare
//    unit-level doMove on the LEADER and can re-pin him for the whole
//    assert window (the historic flake, persistent on slow ticks). doStop
//    detaches the unit from waypoint/formation control, so the doMove is
//    deterministic - and still proves the same thing: a real, commandable
//    AI unit. -----------------------------------------------------------------
gnLdr = leader (gnGrps select 0)
// (bool asserts go through format: triAssert*'s stringifier cannot take a
//  raw GameBool - engine issue, see the parity-test report)
triAssertEq [(format ["%1", alive gnLdr]), "true"]
gnP0 = getPos gnLdr
gnDest = [(gnP0 select 0) + 80, (gnP0 select 1) + 80, 0]
doStop gnLdr
gnLdr doMove gnDest
triSimUntil { ((getPos gnLdr) distance gnP0) > 15 }

// -- nobody died or despawned while moving (no crash, cache stayed put) ------
triAssertGe [(gmGarrisonLive gnOut), 6]

triEndTest
