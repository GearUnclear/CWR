// ============================================================================
//  Showcase.Abel smoke (issue #9, v2): the chapter-based showcase mission
//  boots and every chapter actually drives - and self-verifies - the system
//  it narrates.
//
//  The overlay's SC_QUEUE is driven directly (the exact seam the action menu
//  and the DEMO ALL reel write) with SC_AUTO=true for short narration beats.
//  Per chapter, the test waits on the SC_LASTDONE handshake, asserts the
//  runner's ledger row is PASS, and re-asserts the system-level postcondition
//  independently (treasury delta, unlock set, rank, ownership flip...).
//
//  ORDER IS THE MANIFEST/REEL ORDER and it is load-bearing (README.md):
//  alert (6) runs against the live garrison BEFORE capture (7) wipes it; the
//  alert chapter's own epilogue calms the zone (force-despawn kills the
//  perception sources -> GREEN -> qrf.sqs deletes the QRF) so capture can
//  pass its military-clear check.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }
triSimUntil { !(isNil "SC_READY") }

// menu mounted off the manifest: one action per chapter, non-negative ids
triAssertEq [count SC_ACTS, count SC_CH_IDS]
triAssertGe [SC_ACT_ALL, 0]

// harness mode: short beats
SC_AUTO = true

// --- 0 tour: zones resolved, teleports verified, group parked at the Camp --
SC_QUEUE = SC_QUEUE + [0]
triSimUntil { SC_LASTDONE == 0 }
triAssertEq [(SC_RES select 0), "PASS"]
triAssertLe [((getPos player) distance ((gmZone SC_CAMP) select GM_Z_POS)), 120]

// --- 1 economy: the +500 R grant lands (HR clamps at the manpower cap) -----
scR0 = gmResources
SC_QUEUE = SC_QUEUE + [1]
triSimUntil { SC_LASTDONE == 1 }
triAssertEq [(SC_RES select 1), "PASS"]
triAssertEq [gmResources, scR0 + 500]
triAssertGe [gmManpower, 7]

// --- 2 recruit: a fighter joins through the Camp menu's own request queue --
scN0 = count units (group player)
SC_QUEUE = SC_QUEUE + [2]
triSimUntil { SC_LASTDONE == 2 }
triAssertEq [(SC_RES select 2), "PASS"]
triAssertGe [count units (group player), scN0 + 1]

// --- 3 loot: organic corpse tally + a full unlock threshold of the rifle ---
SC_QUEUE = SC_QUEUE + [3]
triSimUntil { SC_LASTDONE == 3 }
triAssertEq [(SC_RES select 3), "PASS"]
scLW = gmFactionValue [gmResistanceSide, "lootRiflemanWeapon"]
triAssert [scLW in GM_GEAR_UNLOCKED]

// --- 4 companions: the XP bump crosses a threshold on the manager's tick ---
scRk0 = GM_COMP_RANK select 0
SC_QUEUE = SC_QUEUE + [4]
triSimUntil { SC_LASTDONE == 4 }
triAssertEq [(SC_RES select 4), "PASS"]
triAssert [(GM_COMP_RANK select 0) != scRk0]

// --- 5 garrison: live -> banked -> live round-trip through the cache -------
SC_QUEUE = SC_QUEUE + [5]
triSimUntil { SC_LASTDONE == 5 }
triAssertEq [(SC_RES select 5), "PASS"]
triSimUntil { gmGarrisonSpawned SC_OUT }

// --- 6 alert: cover break -> YELLOW -> RED -> QRF -> disengage --------------
triAssert [gmUndercover]
SC_QUEUE = SC_QUEUE + [6]
triSimUntil { SC_LASTDONE == 6 }
triAssertEq [(SC_RES select 6), "PASS"]
triAssert [!gmUndercover]

// --- 7 capture: garrison wiped in-zone -> native ownership flip ------------
SC_QUEUE = SC_QUEUE + [7]
triSimUntil { SC_LASTDONE == 7 }
triAssertEq [(SC_RES select 7), "PASS"]
triAssertEq [((gmZone SC_OUT) select GM_Z_OWNER), gmResistanceSide]

// --- 8 escalation: the forced War Level is overwritten by the ladder -------
SC_QUEUE = SC_QUEUE + [8]
triSimUntil { SC_LASTDONE == 8 }
triAssertEq [(SC_RES select 8), "PASS"]
triAssertLe [gmWarLevel, 2]

// --- 9 persistence: the Save action's own dispatch path saves --------------
SC_QUEUE = SC_QUEUE + [9]
triSimUntil { SC_LASTDONE == 9 }
triAssertEq [(SC_RES select 9), "PASS"]

// --- 10 status: the report renders and the ledger holds no FAIL rows -------
SC_QUEUE = SC_QUEUE + [10]
triSimUntil { SC_LASTDONE == 10 }
triAssertEq [(SC_RES select 10), "PASS"]
triAssert [!("FAIL" in SC_RES)]

triEndTest
