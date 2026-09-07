// Arma 3 right mouse on foot: a quick RMB click toggles the sights (Optics is
// bound to "tap RMB"), while holding RMB drives the temporary zoom (ZoomTemp
// is the plain RMB level) without ever toggling the view.  V still toggles the
// sights on its press edge.
//
// triMouseBtn pushes a real SDL button event through the production funnel,
// so it is timestamped and can tap; triTapWindowMs widens / disables the tap
// window so harness latency cannot turn a tap into a hold or vice versa.  The
// FOV waits use triSimUntil: the zoom easing is wall-clock and headless frames
// are sub-millisecond.  Scancode: V 25.

triSetLanguage "English"
triSimUntil { alive player }
triSimFrames 10
triAssertEq [triCamView, "INTERNAL"]

// ---- 1. Tap RMB toggles the sights (5 s window: the harness release always counts as a tap)
triTapWindowMs 5000
triMouseBtn [1, 1]
triSimFrames 3
triMouseBtn [1, 0]
triSimUntil { triCamView == "GUNNER" }
triAssertEq [triCamView, "GUNNER"]

triMouseBtn [1, 1]
triSimFrames 3
triMouseBtn [1, 0]
triSimUntil { triCamView == "INTERNAL" }
triAssertEq [triCamView, "INTERNAL"]

// ---- 2. Hold RMB zooms and eases back, never toggling (taps off)
triTapWindowMs 0
triSimFrames 5
rmbFov0 = triCamFov
triAssertGt [rmbFov0, 0]
triMouseBtn [1, 1]
triSimUntil { triCamFov < (0.9 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]
triMouseBtn [1, 0]
triSimUntil { triCamFov > (0.95 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]

// ---- 3. V still toggles on the press edge
triKeyDown 25
triSimFrames 3
triKeyUp 25
triSimUntil { triCamView == "GUNNER" }
triAssertEq [triCamView, "GUNNER"]
triKeyDown 25
triSimFrames 3
triKeyUp 25
triSimUntil { triCamView == "INTERNAL" }
triAssertEq [triCamView, "INTERNAL"]

triEndTest
