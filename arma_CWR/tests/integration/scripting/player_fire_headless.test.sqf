// ============================================================================
//  Regression test for engine issue #5: a script issuing `fire` on the local
//  player unit must not crash the process (was: 0xC0000005 in
//  Man::ProcessMoveFunction via the force-fire -> move-queue path, headless).
//
//  Proves the positive behavior too: the forced shot actually leaves the
//  barrel (muzzle ammo count drops), which is the natural way to drive the
//  undercover "fired weapon breaks cover" hook end-to-end.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

guWeap = primaryWeapon player
triAssertNe [guWeap, ""]
guAmmo0 = player ammo guWeap
triAssertGt [guAmmo0, 0]

player fire guWeap

// force-fire latches, aims, and fires within a few sim seconds; the eval
// returning at all proves the process survived (the old crash was immediate)
triSimUntil { (player ammo guWeap) < guAmmo0 }

triEndTest
