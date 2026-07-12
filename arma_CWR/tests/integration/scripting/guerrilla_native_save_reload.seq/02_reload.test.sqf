// ============================================================================
//  Phase 2 of the NATIVE save/reload round-trip (guerrilla_native.abel).
//    Fresh launch of the SAME mission against the SAME shared user dir.
//    init.sqs re-seeds the defaults first (proving a clean slate), then
//    triLoadGame rehydrates GGameState AND the native guerrilla subsystems
//    from phase 1's save. Diffed here:
//      * script globals (gmResources/gmManpower/gmWarLevel/GM_COMP_XP/
//        GM_GEAR_UNLOCKED) - GGameState serialization;
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

// -- restore phase 1's binary save from the shared UserDir/Saved/Tmp/gnat.fps -
triAssertEq [(triLoadGame "gnat"), "OK"]
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
// mid-consolidation progress survives the reload (inert on a resistance-owned
// zone, so no tick can move it before this assert)
triAssertEq [((gmZone gsOut) select 9), 50]

// -- the natively-SERIALIZED campaignLoaded handler fired (no script re-armed
//    anything) and campaign.sqs consumed it: GM_pVer = the save version ------
triSimUntil { not (isNil "GM_pVer") }
triAssertGe [GM_pVer, 1]

// -- companion reconciliation: a live body, restored or rebuilt ---------------
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triAssertEq [(format ["%1", alive (GM_COMP_OBJ select 0)]), "true"]
triAssertEq [(format ["%1", GM_COMP_ALIVE select 0]), "true"]

// -- DIFF: civilian-layer ledgers ride GGameState (issue #8) -------------------
triAssertEq [(format ["%1", "Village" in GM_PANIC_ZONES]), "true"]
triAssertEq [(count GM_PANIC_UNTIL), 1]
triAssertEq [(GM_RESENT_ZONES select 0), "Village"]
triAssertEq [(GM_RESENT_AMT select 0), 7]
triAssertEq [gmDayCount, 3]

// -- managers keep ticking after the load (handle-prune works): both loop
//    counters advance, and the consumer drains the stamped NULL-handle kill
//    record without error (isNull killer -> OTHER, but the far pos takes the
//    effect-radius drop - no crash, no wedge, no ledger side effects) ---------
gsT2 = gmCivTicks
triSimUntil { gmCivTicks > gsT2 }
gsT3 = gmShkTicks
triSimUntil { gmShkTicks > gsT3 }
triSimUntil { (count gmCivKilled) == 0 }

triEndTest
