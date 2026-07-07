// ============================================================================
//  Showcase.Abel smoke (issue #9): the systems-showcase mission boots and
//  every rapid-access demo actually drives the system it displays.
//
//  The overlay's SC_REQ/SC_PENDING queue is driven directly - the exact seam
//  showcase/action.sqs (the addAction dispatcher) writes - and SC_LASTDONE is
//  the per-demo completion handshake run.sqs raises. Demo order matters:
//  capture wipes the Outpost garrison BEFORE the cover-break demo teleports
//  there, so the player never stands uncovered next to live occupiers (the
//  engine consumes a break latch with no garrison around; verified against
//  AlertMachine::EvaluateAlert).
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }
triSimUntil { !(isNil "SC_READY") }

// menu mounted: addAction ids are non-negative handles
triAssertGe [SC_ACT_HUD, 0]
triAssertGe [SC_ACT_TIME, 0]

// --- economy: one press grants +500 R (HR clamps at the manpower cap) ------
scR0 = gmResources
SC_REQ = SC_ACT_GRANT
SC_PENDING = true
triSimUntil { SC_LASTDONE == SC_ACT_GRANT }
triAssertEq [gmResources, scR0 + 500]
triAssertGe [gmManpower, 7]

// --- loot/gear: one press tallies a full unlock threshold of the rifle -----
SC_REQ = SC_ACT_LOOT
SC_PENDING = true
triSimUntil { SC_LASTDONE == SC_ACT_LOOT }
scLW = gmFactionValue [gmResistanceSide, "lootRiflemanWeapon"]
triAssert [scLW in GM_GEAR_UNLOCKED]

// --- companions: +160 XP crosses the SERGEANT threshold (100 -> 260) -------
SC_REQ = SC_ACT_XP
SC_PENDING = true
triSimUntil { SC_LASTDONE == SC_ACT_XP }
triAssertGe [(GM_COMP_XP select 0), 260]
triSimUntil { (GM_COMP_RANK select 0) == "SERGEANT" }

// --- teleport cycle: first press lands the group at the Village ------------
SC_REQ = SC_ACT_TP
SC_PENDING = true
triSimUntil { SC_LASTDONE == SC_ACT_TP }
triAssertLe [((getPos player) distance ((gmZone SC_VILL) select GM_Z_POS)), 120]

// --- garrison cache: force despawn banks the reserve; the cache respawns it
//     (the Village sits ~535 m from the Outpost, inside the 800 m cacheRadius)
triSimUntil { gmGarrisonSpawned SC_OUT }
triSimUntil { (gmGarrisonLive SC_OUT) > 0 }
SC_REQ = SC_ACT_DESPAWN
SC_PENDING = true
triSimUntil { SC_LASTDONE == SC_ACT_DESPAWN }
triSimUntil { gmGarrisonSpawned SC_OUT }
triSimUntil { (gmGarrisonLive SC_OUT) > 0 }

// --- capture: wipe the garrison with the player present -> native flip -----
SC_REQ = SC_ACT_CAPTURE
SC_PENDING = true
triSimUntil { SC_LASTDONE == SC_ACT_CAPTURE }
triSimUntil { ((gmZone SC_OUT) select GM_Z_OWNER) == gmResistanceSide }

// --- alert machine: the cover-break latch is consumed on the next tick -----
triAssert [gmUndercover]
SC_REQ = SC_ACT_ALERT
SC_PENDING = true
triSimUntil { SC_LASTDONE == SC_ACT_ALERT }
triSimUntil { !gmUndercover }

triEndTest
