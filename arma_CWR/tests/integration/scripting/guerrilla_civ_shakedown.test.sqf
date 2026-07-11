// ============================================================================
//  Guerrilla Mode civilian layer - shakedown scene + resentment (issue #8).
//    shakedown.sqs is an abstract-scene director (mod-plan 14: halt, face,
//    hold, text beats, outcome as state change - zero new animations).
//    Forced deterministically via the contract's debug hooks: setting
//    GM_SHK_FORCEKILL then GM_SHK_FORCE makes the next director tick skip
//    the chance/cooldown/range gates, launch a scene at the nearest
//    POPULATED CITY, and resolve the outcome as the public killing
//    (both flags consumed by the manager). Proven on full CWA data (Abel):
//      1. scene observables: GM_SHK_ACTIVE, a 2-man initiator pair
//         (GM_SHK_GRP), a live victim (GM_SHK_VICTIM);
//      2. the execution lands (doTarget/doWatch + setDammage beat);
//      3. the SYNTHETIC kill record (posted with the TRUE initiator -
//         setDammage has no killer attribution) flows through
//         civilians.sqs's consumer: Village support DROPS (OCCUPIER
//         classification, GM_CIV_SUP_OCC);
//      4. resentment scheduled: the GM_RESENT_* ledger gains a row keyed by
//         the zone NAME;
//      5. resentment applied: with GM_RESENT_DELAY shrunk to 10 s the
//         shakedown ticker drains the due row and support comes back UP
//         (the delayed repression-radicalizes tick);
//      6. the scene winds down (GM_SHK_ACTIVE false, hooks consumed).
//
//  Determinism: the walk phase is collapsed by setPos-ing the initiator
//  pair onto the victim (the arrival gate is a distance check), so no AI
//  pathing sits on the critical path; the kill roll is forced - no random
//  rolls are relied on anywhere. The support BASELINE is pinned first: the
//  native ZoneRegistry tick raises CITY support by supportRate every
//  tickInterval while a GUER unit (the parked player) stands inside
//  zoneArea, but ONLY while the zone owner is still NEUTRAL
//  (ZoneRegistry::EvaluateTick) - so the test sets support past supportFlip
//  and waits for the NEUTRAL->GUER flip BEFORE recording gcSup0. Drift is
//  then off for good and the consumer's -8 must undercut the baseline.
//  (A CITY flip is reaction-free apart from a hint: capture.sqs spawns hold
//  garrisons for military zones only, and the forced scene picker filters
//  on CITY+populated, not owner - verified, both.)
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_CIV_READY") }
triSimUntil { not (isNil "gmCivTicks") }
// shakedown.sqs seeds the force hooks to false AFTER its READY sentinel -
// wait for the seed itself, or a test-side write could be silently clobbered
triSimUntil { not (isNil "GM_SHK_FORCE") }

gcVil = gmZoneIndex "Village"
triAssertGe [gcVil, 0]

// -- populate the Village: player at the centre, wait for civ groups ----------
gcPos = (gmZone gcVil) select 8
player setPos [gcPos select 0, gcPos select 1, 0]
triSimUntil { (count GM_CIV_GROUPS) > 0 }

// -- shrink the resentment clock BEFORE forcing (tunables read at call time) --
GM_RESENT_DELAY = 10

// -- kill the native GUER-presence support drift BEFORE baselining: pin
//    support past supportFlip (60) and wait for the NEUTRAL->GUER ownership
//    flip - the drift branch only runs while the owner is NEUTRAL, so the
//    baseline below can never go stale under the scene's ~40-60 s runtime ----
gmZoneSet [gcVil, "support", 80]
triSimUntil { ((gmZone gcVil) select 2) == gmResistanceSide }

// -- baseline support (stable: drift is off, only the consumer writes now) ----
gcSup0 = (gmZone gcVil) select 4

// -- force the deterministic public killing: the KILL flag FIRST - the scene
//    launch consumes GM_SHK_FORCE and must already see GM_SHK_FORCEKILL ------
GM_SHK_FORCEKILL = true
GM_SHK_FORCE = true

// -- scene observables ------------------------------------------------------------
triSimUntil { GM_SHK_ACTIVE }
triAssertEq [(count units GM_SHK_GRP), 2]
triAssertEq [(format ["%1", isNull GM_SHK_VICTIM]), "false"]

// -- collapse the walk phase: teleport the pair onto the victim (arrival is a
//    distance gate; GM_SHK_WALK_T stays untouched, nothing can time out) ------
"_x setPos (getPos GM_SHK_VICTIM)" forEach (units GM_SHK_GRP)

// -- the execution landed ----------------------------------------------------------
triSimUntil { not (alive GM_SHK_VICTIM) }

// -- consumer effects: support dropped via the synthetic record's OCCUPIER
//    classification (civilians.sqs drains gmCivKilled on its ~5 s tick) -------
triSimUntil { ((gmZone gcVil) select 4) < gcSup0 }

// -- resentment scheduled (ledger keyed by zone NAME, never index/handle) ------
triAssertGe [count GM_RESENT_ZONES, 1]
triAssertEq [(GM_RESENT_ZONES select 0), "Village"]
gcSup1 = (gmZone gcVil) select 4

// -- resentment applied: the ticker re-resolves the name, adds the +tick and
//    removes the row ---------------------------------------------------------------
triSimUntil { (count GM_RESENT_ZONES) == 0 }
triSimUntil { ((gmZone gcVil) select 4) > gcSup1 }

// -- scene wound down ---------------------------------------------------------------
triSimUntil { not GM_SHK_ACTIVE }

triEndTest
