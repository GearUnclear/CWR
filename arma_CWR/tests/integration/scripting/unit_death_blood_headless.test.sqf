// ============================================================================
//  Regression test for engine issues #5/#6: the actual 0xC0000005 crash was
//  the MFDead branch of Man::ProcessMoveFunction spawning blood-slop on unit
//  death and dereferencing a NULL Preloaded(SlopBlood) shape (null headless
//  under the --no-strict preloader). The player-fire test proves the *fire*
//  path survives; this proves the *death* path does too - the branch that the
//  minidumps actually faulted in.
//
//  Killing the whole Outpost garrison drives every unit through the die move
//  -> MFDead. blood defaults on (RuntimeFlags.blood = true), so the 30% slop
//  roll fires across six deaths (P(>=1 roll) ~= 0.88); pre-fix that dereferences
//  null and crashes. Reaching triEndTest proves the process survived.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

// the GarrisonCache auto-spawns the Outpost garrison (600 m from the Camp,
// inside the 800 m cacheRadius) - same setup as guerrilla_native_spawn
udOut = gmZoneIndex "Outpost"
triAssertGe [udOut, 0]
triSimUntil { gmGarrisonSpawned udOut }
triSimUntil { (gmGarrisonLive udOut) >= 6 }

udUnits = units ((gmGarrisonGroups udOut) select 0)
triAssertGe [(count udUnits), 1]

// kill the garrison; each death queues an MFDead context whose animation
// completion rolls the blood-slop spawn
{_x setDamage 1} forEach udUnits

// step well past the die-animation length so every MFDead branch executes;
// frame-bounded (not condition-bounded) so it can never hang
triSimFrames 300

// survived the death path with blood enabled -> the null-shape guard held
triSimUntil { (gmGarrisonLive udOut) == 0 }
triEndTest
