// Arma 3 right mouse: press zooms immediately, a quick release toggles sights,
// and a hold only zooms. Cover rifle, LAW, and tank driver/gunner cameras.
//
// triMouseBtn pushes a real SDL button event through the production funnel,
// so it is timestamped and can tap. Tap cases widen the window to tolerate
// harness latency; hold cases use the production 250 ms window and wait at
// least 350 ms before release. FOV easing uses wall-clock time, so pumping a
// few headless frames alone is insufficient. Scancode: V 25.

triSetLanguage "English"
triSimUntil { alive player }
triSimFrames 10
triAssertEq [triCamView, "INTERNAL"]

// ---- 1. Press zooms BEFORE the tap window expires; only release enters sights
triTapWindowMs 2000
rmbFov0 = triCamFov
triAssertGt [rmbFov0, 0]
triMouseBtn [1, 1]
triSimUntil { triCamFov < (0.9 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]
triMouseBtn [1, 0]
triSimUntil { triCamView == "GUNNER" }
triAssertEq [triCamView, "GUNNER"]

// A real hold inside the sights changes neither the camera nor its FOV,
// including on release. Sample after a settled frame, not during entry easing.
triTapWindowMs 250
triWait 350
triSimFrames 10
rmbOpticsFov = triCamFov
triAssertGt [rmbOpticsFov, 0]
triMouseBtn [1, 1]
triSimFrames 3
triWait 350
triSimFrames 10
triAssertEq [triCamView, "GUNNER"]
triAssertNear [triCamFov, rmbOpticsFov, 0.001]
triMouseBtn [1, 0]
triSimFrames 10
triAssertEq [triCamView, "GUNNER"]
triAssertNear [triCamFov, rmbOpticsFov, 0.001]

triTapWindowMs 2000
triMouseBtn [1, 1]
triSimFrames 3
triAssertEq [triCamView, "GUNNER"]
triMouseBtn [1, 0]
triSimUntil { triCamView == "INTERNAL" }
triAssertEq [triCamView, "INTERNAL"]

// ---- 2. Hold RMB zooms and eases back, never toggling at the normal threshold
triTapWindowMs 250
triWait 350
triSimFrames 10
rmbFov0 = triCamFov
triAssertGt [rmbFov0, 0]
triMouseBtn [1, 1]
triSimFrames 3
triWait 350
triSimUntil { triCamFov < (0.9 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]
triMouseBtn [1, 0]
triSimUntil { triCamFov > (0.95 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]

// ---- 3. V still toggles on the press edge
triKeyDown 25
triSimFrames 3
triSimUntil { triCamView == "GUNNER" }
triAssertEq [triCamView, "GUNNER"]
triKeyUp 25
triSimFrames 3
triAssertEq [triCamView, "GUNNER"]
triKeyDown 25
triSimFrames 3
triSimUntil { triCamView == "INTERNAL" }
triAssertEq [triCamView, "INTERNAL"]
triKeyUp 25
triSimFrames 3
triAssertEq [triCamView, "INTERNAL"]

// ---- 4. A launcher must not veto temporary zoom
// Remove the rifle so a failed selection cannot accidentally test it again.
removeAllWeapons player
player addMagazine "LAWLauncher"
player addWeapon "LAWLauncher"
player selectWeapon "LAWLauncher"
triAssertEq [weapons player, ["LAWLauncher"]]
rmbSettle = time
triSimUntil { time > rmbSettle + 3 }
triAssertEq [triCamView, "INTERNAL"]
rmbFov0 = triCamFov
triAssertGt [rmbFov0, 0]
triMouseBtn [1, 1]
triSimFrames 3
triWait 350
triSimUntil { triCamFov < (0.9 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]
triMouseBtn [1, 0]
triSimUntil { triCamFov > (0.95 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]

// ---- 5. Vehicle driver: the same hold zoom and release recovery
// Use open dry ground, away from the original fixture's coastal buildings.
player setPos [7500, 5700, 0]
rmbTank = "M60" createVehicle [7510, 5700, 0]
triAssertEq [typeOf rmbTank, "M60"]
player moveInDriver rmbTank
triSimUntil { (driver rmbTank) == player }
triSimFrames 10
triAssertEq [triCamView, "INTERNAL"]
rmbFov0 = triCamFov
triAssertGt [rmbFov0, 0]
triMouseBtn [1, 1]
triSimFrames 3
triWait 350
triSimUntil { triCamFov < (0.9 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]
triMouseBtn [1, 0]
triSimUntil { triCamFov > (0.95 * rmbFov0) }
triAssertEq [triCamView, "INTERNAL"]

// ---- 6. Vehicle gunner: tap enters/exits optics; holding in optics does nothing
deleteVehicle rmbTank
triSimUntil { (vehicle player) == player }
rmbTank = "M60" createVehicle [7510, 5700, 0]
triAssertEq [typeOf rmbTank, "M60"]
player moveInGunner rmbTank
triSimUntil { (gunner rmbTank) == player }
triSimFrames 10
triAssertEq [triCamView, "INTERNAL"]
triTapWindowMs 2000
triMouseBtn [1, 1]
triSimFrames 3
triAssertEq [triCamView, "INTERNAL"]
triMouseBtn [1, 0]
triSimUntil { triCamView == "GUNNER" }
triAssertEq [triCamView, "GUNNER"]

triTapWindowMs 250
triWait 350
triSimFrames 10
rmbOpticsFov = triCamFov
triAssertGt [rmbOpticsFov, 0]
triMouseBtn [1, 1]
triSimFrames 3
triWait 350
triSimFrames 10
triAssertEq [triCamView, "GUNNER"]
triAssertNear [triCamFov, rmbOpticsFov, 0.001]
triMouseBtn [1, 0]
triSimFrames 10
triAssertEq [triCamView, "GUNNER"]
triAssertNear [triCamFov, rmbOpticsFov, 0.001]

triTapWindowMs 2000
triMouseBtn [1, 1]
triSimFrames 3
triAssertEq [triCamView, "GUNNER"]
triMouseBtn [1, 0]
triSimUntil { triCamView == "INTERNAL" }
triAssertEq [triCamView, "INTERNAL"]
triTapWindowMs 250
deleteVehicle rmbTank
triSimUntil { (vehicle player) == player }

triEndTest
