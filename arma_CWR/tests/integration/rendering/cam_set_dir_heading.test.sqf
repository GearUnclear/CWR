// camSetDir must TURN the camera, not pitch it.
//
// The command table wired "camSetDir" and "camSetBank" to the CamSetDive
// handler (GameStateExt.cpp), so a vanilla OFP cutscene line like
// `_cam camSetDir 180` fed its argument to SetDive and left the heading
// untouched.  The correct CamSetDir / CamSetBank handlers existed but were
// unreachable dead code.
//
// Committing an angle also has to reach the renderer: CameraVehicle derives
// its orientation from the camera target, so CameraVehicle::Commit turns the
// heading/dive pair into a target at virtual infinity.  Before that the angle
// was stored on the CameraHolder and never read by anything.
//
// Broken-state delta: with the alias in place the camera never leaves its
// initial heading, so the getDir assertions read ~0 instead of 90/270 and the
// frame is identical between the two headings.
//
// Heading convention matches getDir: degrees clockwise from north, in [0,360).
// 90 and 270 are used rather than 0/180 so the assertions stay clear of the
// getDir wrap at 0.
//
// HARNESS NOTE: Trident sends each statement as a separate eval, so `_local`
// variables do not survive from one line to the next — the camera is a global.

triSimUntil { time >= 1 }
triSimUntil { alive player }

// Keep the player out of frame; the mission spawns them at the camera spot.
player setPos [8151.0, 3100.0, 0.0]
setViewDistance 1500
0 setOvercast 0.0
0 setFog 0.0

cwrCam = "camera" camCreate [8151.0, 3148.0, 30.0]
cwrCam cameraEffect ["internal", "back"]
cwrCam camSetFov 0.7

// Tilt down so the lower half of the frame is terrain rather than flat sky —
// two opposite headings have to differ somewhere for the pixel check below.
cwrCam camSetDive -15
cwrCam camCommit 0
triSimFrames 30

// --- east -------------------------------------------------------------------
cwrCam camSetDir 90
cwrCam camCommit 0
triSimFrames 30
triAssertNear [(getDir cwrCam), 90, 1]
triScreenshot "00_heading_east"
triPixelLatch [0.5, 0.72]

// --- west: same camera, same position, opposite heading ----------------------
cwrCam camSetDir 270
cwrCam camCommit 0
triSimFrames 30
triAssertNear [(getDir cwrCam), 270, 1]
triScreenshot "01_heading_west"
triAssertPixelChanged [0.5, 0.72, 8]

// --- camSetDive is a different command --------------------------------------
// Under the aliased table these were indistinguishable.  Pitching the camera
// must leave the heading where camSetDir put it.
cwrCam camSetDive -30
cwrCam camCommit 0
triSimFrames 30
triAssertNear [(getDir cwrCam), 270, 1]

// --- camSetBank does not steer either ----------------------------------------
cwrCam camSetBank 15
cwrCam camCommit 0
triSimFrames 30
triAssertNear [(getDir cwrCam), 270, 1]

// --- a later camSetTarget still wins over the angles --------------------------
// The angle path writes through the same target slot camSetTarget uses, so the
// two must not fight across commits: aiming due south of the camera swings the
// heading to 180.  (Due north would sit on the getDir wrap and read either
// ~0 or ~360, so south is the unambiguous direction to check.)
cwrCam camSetTarget [8151.0, 2648.0, 30.0]
cwrCam camCommit 0
triSimFrames 30
triAssertNear [(getDir cwrCam), 180, 1]

cwrCam cameraEffect ["terminate", "back"]
triEndTest
