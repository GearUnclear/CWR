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
triAssertEq [triWindowFocus false, true]
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triAssertEq [triWindowFocus true, true]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
// Both events may arrive between simulation steps while the window is paused.
// The epoch must still invalidate the held key even though focus is restored.
triAssertEq [triWindowFocus false, triWindowFocus true]
leanSettle = time
triSimUntil { time > leanSettle + 1 }
triAssertNear [(triLeanPose player) select 0, 0, 0.01]
triKeyUp 8
triSimFrames 3
triKeyDown 8
triSimUntil { ((triLeanPose player) select 0) > 0.95 }
triKeyUp 8
triSimUntil { abs ((triLeanPose player) select 0) < 0.01 }
triEndTest
