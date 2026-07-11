// ============================================================================
//  Guerrilla Mode civilian layer - panic episode + economy skip (issue #8).
//    A public killing panics the town: civilians scatter, the zone empties
//    and the spawner is suppressed until the episode expires; a panicked
//    town pays NO manpower and halved Roubles for the episode (the
//    economy.sqs coupling). Forced deterministically via GM_SHK_FORCEKILL +
//    GM_SHK_FORCE. Proven on full CWA data (Abel):
//      1. baseline: Village support raised to 80 - the ONLY town at/above
//         GM_ECON_HR_SUP_MIN (seeded cities sit at 20) - and the economy
//         tick shrunk, so a normal tick pays gmEcoHR > 0;
//      2. the forced killing lands -> "Village" enters GM_PANIC_ZONES;
//      3. displacement/empty: with the player still standing in town, the
//         civ group count drains to 0 (empty-phase despawn + spawner
//         suppression - the non-flaky displacement observable; per-unit run
//         distances are deliberately NOT asserted, the doMove displacement
//         assert in guerrilla_native_spawn is the known-flaky precedent);
//      4. economy skip: a FRESH economy tick computed inside the episode
//         pays gmEcoHR == 0 (panicked Village is the only HR-qualifying
//         town; the kill's -8 leaves it at 72, still >= 50, so the skip is
//         attributable to panic alone);
//      5. recovery: the ledger row expires and the spawner repopulates.
//
//  Tunables are shrunk test-side BEFORE forcing (all read at use time):
//  panic episode 10+40 s, economy tick 15 s -> at least two economy ticks
//  fit inside the episode by construction.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_CIV_READY") }
triSimUntil { not (isNil "gmCivTicks") }
triSimUntil { not (isNil "GM_SHK_FORCE") }

gcVil = gmZoneIndex "Village"
triAssertGe [gcVil, 0]

// -- make Village the only HR-qualifying town ----------------------------------
gmZoneSet [gcVil, "support", 80]

// -- shrink the clocks BEFORE forcing (tunables read at use time) --------------
GM_PANIC_SCATTER_T = 10
GM_PANIC_EMPTY_T = 40
GM_ECON_TICK = 15

// -- populate the Village ---------------------------------------------------------
gcPos = (gmZone gcVil) select 8
player setPos [gcPos select 0, gcPos select 1, 0]
triSimUntil { (count GM_CIV_GROUPS) > 0 }

// -- baseline economy: a normal tick pays HR (Village 80 >= 50, not panicked) --
triSimUntil { not (isNil "gmEcoHR") }
triSimUntil { gmEcoHR > 0 }

// -- force the killing (KILL flag FIRST, then the scene trigger); collapse the
//    walk phase as in guerrilla_civ_shakedown ----------------------------------
GM_SHK_FORCEKILL = true
GM_SHK_FORCE = true
triSimUntil { GM_SHK_ACTIVE }
"_x setPos (getPos GM_SHK_VICTIM)" forEach (units GM_SHK_GRP)

// -- panic flag: the kill-queue consumer panicked the town ---------------------
triSimUntil { "Village" in GM_PANIC_ZONES }

// -- displacement/empty: groups drain with the player still in town ------------
triSimUntil { (count GM_CIV_GROUPS) == 0 }

// -- economy skip: overwrite the last tick's result with a sentinel, wait for
//    a FRESH tick computed inside the episode - it must pay zero HR ------------
gmEcoHR = -1
triSimUntil { gmEcoHR >= 0 }
triAssertEq [gmEcoHR, 0]

// -- recovery: the episode (10+40 s) expires, the spawner repopulates ----------
triSimUntil { not ("Village" in GM_PANIC_ZONES) }
triSimUntil { (count GM_CIV_GROUPS) > 0 }

triEndTest
