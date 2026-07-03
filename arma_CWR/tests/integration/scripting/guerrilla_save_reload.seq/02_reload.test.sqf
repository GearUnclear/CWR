// ============================================================================
//  Phase 2 of the guerrilla save/reload round-trip (NATIVE engine surface).
//    Fresh launch of the SAME mission against the SAME shared user dir.
//    init.sqs re-seeds the defaults first (proving a clean slate), then
//    triLoadGame rehydrates GGameState AND the native guerrilla subsystems
//    from phase 1's save. Diffed here:
//      * script globals - GGameState serialization;
//      * native zone state written via gmZoneSet (owner/income/support) -
//        ZoneRegistry::Serialize, matched by zone NAME across the config
//        rebuild the load performs;
//      * the campaignLoaded EVENT HANDLER round-trip: the engine queues the
//        event on the first tick after the load, the SERIALIZED handler
//        enqueues it, and campaign.sqs's manager consumes it (GM_pVer);
//      * companion reconciliation: campaign.sqs verifies/nulls the restored
//        handle and companions.sqs rebuilds - a live Petra either way.
// ============================================================================

triSimUntil { GM_LIB_READY }
gsOut = gmZoneIndex "Outpost"
gsVil = gmZoneIndex "Village"

// -- baseline: fresh-boot defaults, NOT the sentinels (guards against a false
//    pass from leftover state in the shared user dir) -------------------------
triAssertEq [gmResources, 100]
triAssertEq [((gmZone gsOut) select 2), gmOccupierSide]

// -- restore phase 1's binary save from the shared UserDir/Saved/Tmp/grr.fps --
triAssertEq [(triLoadGame "grr"), "OK"]
triSimFrames 3

// -- DIFF: script-global layer -------------------------------------------------
triAssertEq [gmResources, 777]
triAssertEq [gmManpower, 9]
triAssertEq [gmWarLevel, 5]
triAssertEq [(GM_COMP_XP select 0), 12345]
triAssertEq [(format ["%1", "AK47" in GM_GEAR_UNLOCKED]), "true"]

// -- DIFF: native zone-registry layer (name-matched across the load) ----------
triAssertEq [((gmZone gsOut) select 2), gmResistanceSide]
triAssertEq [((gmZone gsOut) select 5), 99]
triAssertEq [((gmZone gsVil) select 4), 55]

// -- the natively-SERIALIZED campaignLoaded handler fired (no script re-armed
//    anything) and campaign.sqs consumed it: GM_pVer = the save version ------
triSimUntil { not (isNil "GM_pVer") }
triAssertGe [GM_pVer, 1]

// -- companion reconciliation: a live body, restored or rebuilt ---------------
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triAssertEq [(format ["%1", alive (GM_COMP_OBJ select 0)]), "true"]
triAssertEq [(format ["%1", GM_COMP_ALIVE select 0]), "true"]

triEndTest
