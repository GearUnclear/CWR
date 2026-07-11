// ============================================================================
//  Guerrilla Mode civilian layer - ambient town population (issue #8).
//    civilians.sqs is a SCRIPTED player-distance cache (the native
//    GarrisonCache's philosophy, not its code): standing inside
//    GM_CIV_RADIUS of a CITY zone spawns small CIV-side wander groups;
//    walking beyond GM_CIV_DESPAWN_R despawns them. Proven here on full CWA
//    data (Abel), against the issue-8 contract:
//      1. boot sentinel GM_CIV_READY comes up (CIV descriptor + CITY zones);
//      2. player at the Village (Houdan) centre -> the spawner populates it;
//      3. spawned units are CIV-side (engine renders the side name "CIV")
//         and placed near the town centre;
//      4. budget caps hold across manager ticks: global GM_CIV_MAX_GROUPS
//         and per-group GM_CIV_GROUP_SIZE (the "~63-group CIV budget" is
//         folklore - these are DESIGN budgets, which is what we pin);
//      5. player far offshore (> GM_CIV_DESPAWN_R from every CITY) -> the
//         cache drains: count GM_CIV_GROUPS reaches 0.
//
//  Determinism: no random rolls on the critical path - spawn/despawn are
//  pure player-distance gates on the ~5 s manager tick.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_CIV_READY") }
// GM_CIV_READY lands BEFORE the tunable/state seeds in civilians.sqs;
// gmCivTicks is seeded after them - wait for it so nothing the test reads
// (or a sibling test overrides) can be clobbered by a late seed.
triSimUntil { not (isNil "gmCivTicks") }

gcVil = gmZoneIndex "Village"
triAssertGe [gcVil, 0]

// -- stand the player at the Village centre: inside GM_CIV_RADIUS -------------
gcPos = (gmZone gcVil) select 8
player setPos [gcPos select 0, gcPos select 1, 0]

// -- the spawner ran ------------------------------------------------------------
triSimUntil { (count GM_CIV_GROUPS) > 0 }

// -- spawned near the town centre (jittered spawn + 80 m wander radius; 300 m
//    is the contract's own effect radius - a safely generous bound) -----------
triAssertGe [300, ((getPos (leader (GM_CIV_GROUPS select 0))) distance gcPos)]

// -- CIV side --------------------------------------------------------------------
triAssertEq [(format ["%1", side ((units (GM_CIV_GROUPS select 0)) select 0)]), "CIV"]

// -- let a couple of full manager ticks pass, then pin the budget caps ---------
gcT0 = gmCivTicks
triSimUntil { gmCivTicks > (gcT0 + 1) }
triAssertGe [GM_CIV_MAX_GROUPS, count GM_CIV_GROUPS]
triAssertGe [GM_CIV_GROUP_SIZE, count units (GM_CIV_GROUPS select 0)]

// -- despawn: the SW offshore corner is far (>> GM_CIV_DESPAWN_R) from every
//    Abel town, seeded CITY zones included --------------------------------------
player setPos [500, 500, 0]
triSimUntil { (count GM_CIV_GROUPS) == 0 }

triEndTest
