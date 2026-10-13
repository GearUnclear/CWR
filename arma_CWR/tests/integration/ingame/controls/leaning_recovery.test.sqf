triSetLanguage "English"
triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 2 }
// A desktop window that is not in the foreground reports focus loss, which
// correctly suspends leaning. Claim focus the way a real click would.
triWindowFocus true
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
// Pausing can stop simulation, so context changes must still invalidate lean.
triSendKey 41
triWaitFrames 5
triAssertNe [triDisplay, 46]
triSendKey 41
triSimUntil { triDisplay == 46 }
triSimFrames 5
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
// A driver never inherits the on-foot body offset.
leanTank = "M60" createVehicle [7510,5700,0]
player moveInDriver leanTank
triSimUntil { (driver leanTank) == player }
triSimFrames 10
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
deleteVehicle leanTank
triSimUntil { (vehicle player) == player }
leanSettle = time
triSimUntil { time > leanSettle + 3 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
// Sprint interrupts lean and requires a fresh lean press afterward.
triKeyDown 225
triKeyDown 26
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triKeyUp 225
triKeyUp 26
leanSettle = time
triSimUntil { time > leanSettle + 2 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
// Lean is transient across binary save/load, even when the key stays held.
triAssertEq [triSaveGame "leaning", "OK"]
triAssertEq [triLoadGame "leaning", "OK"]
triSimUntil { alive player }
triSimFrames 5
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
// Slow walking keeps the pose active; it does not move the root by itself.
triKeyDown 9
triSimFrames 3
triKeyUp 9
triKeyDown 26
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertGt [(triLeanPose player) select 11, 0.1]
triAssertGt [(triLeanPose player) select 0, 0.95]
triKeyUp 26
triKeyDown 9
triSimFrames 3
triKeyUp 9
leanSettle = time
triSimUntil { time > leanSettle + 1 }
// Reloading interrupts an active lean; completion cannot resurrect a held key.
triFirePlayerWeapon
triKeyDown 21
triSimFrames 3
triKeyUp 21
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
leanSettle = time
triSimUntil { time > leanSettle + 5 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
// Launcher poses cannot inherit a rifle lean.
player addMagazine "LAWLauncher"
player addWeapon "LAWLauncher"
player selectWeapon "LAWLauncher"
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
leanSettle = time
triSimUntil { time > leanSettle + 3 }
player selectWeapon "M16"
leanSettle = time
triSimUntil { time > leanSettle + 4 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
player setDammage 1
triSimUntil { !(alive player) }
triSimFrames 10
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triEndTest
