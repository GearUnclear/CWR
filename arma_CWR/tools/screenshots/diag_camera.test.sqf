// Camera-framing diagnostic #2 for the @LoBo showcase.
//
// Settled by diag #1: triSetView [east, UP, north, dirE, dirUp, dirN] frames
// exactly, and every coordinate can be computed in-script from
// `getPosASL obj` -> [east, north, ASL], so no terrain heights need hardcoding.
// Open question here: the gameplay HUD (ammo counter, action menu, crosshair)
// was still drawn over the frame.  DisplayUIMenus.cpp:974 gates the UI on
// `!GWorld->GetCameraEffect()`, so an active cutscene camera should suppress
// it - this checks that a cameraEffect camera and a triSetView override
// coexist, and picks a time of day.
//
// LABEL RULE (learned the hard way in diag #1): triScreenshot must be given a
// STRING LITERAL.  The runner parses the literal out of the statement to keep
// its own sequence counter, so a `format [...]` label desyncs it and the run
// dies on FAIL:screenshot_not_written.

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

setViewDistance 3200
0 setOvercast 0.05
player setPos [7900, 3600, 0]
player setCaptive true

ssHero = "LoBoMerkava2" createVehicle [7900, 3200, 0]
ssHero setDir 40
ssHero setCaptive true
triSimFrames 60

// -- aim helper: place the camera at ssOff = [dEast, dUp, dNorth] from ssTgt
//    and look at a point ssAimH metres above the target's origin ---------------
ssAim = {
    ssT = getPosASL ssTgt;
    ssCx = (ssT select 0) + (ssOff select 0);
    ssCy = (ssT select 2) + (ssOff select 1);
    ssCz = (ssT select 1) + (ssOff select 2);
    triSetView [ssCx, ssCy, ssCz, (ssT select 0) - ssCx, ((ssT select 2) + ssAimH) - ssCy, (ssT select 1) - ssCz]
}

ssTgt = ssHero
ssAimH = 1.5

// -- A: no camera effect (baseline, expect HUD) ------------------------------
setDate [1985, 6, 15, 15, 0]
ssOff = [-7, 3, -7]
call ssAim
triSimFrames 12
triScreenshot "A_nocameffect_1500"

// -- B: cutscene camera active for its HUD-suppressing side effect only ------
ssCam = "camera" camCreate [7900, 3200, 30]
ssCam cameraEffect ["internal", "back"]
triSimFrames 12
call ssAim
triSimFrames 12
triScreenshot "B_cameffect_1500"

// -- C: closer, lower - the hero framing the shoot actually wants ------------
ssOff = [-4.5, 1.4, -5]
call ssAim
triSimFrames 8
triScreenshot "C_cameffect_close"

// -- D..F: time of day -------------------------------------------------------
setDate [1985, 6, 15, 12, 0]
triSimFrames 20
triScreenshot "D_noon"

setDate [1985, 6, 15, 17, 30]
triSimFrames 20
triScreenshot "E_evening"

setDate [1985, 6, 15, 7, 30]
triSimFrames 20
triScreenshot "F_morning"

// -- G: does the view override survive a scene change / is it sticky? --------
setDate [1985, 6, 15, 15, 0]
ssOff = [-12, 6, -12]
call ssAim
triSimFrames 12
triScreenshot "G_wide"

// -- H: infantry framing, and do resistance/civilian centers exist? ----------
ssG = createGroup resistance
"LoBo_Terror_01R" createUnit [[7906, 3196, 0], ssG, "", 0.6, "PRIVATE"]
triSimFrames 90
triScreenshot "H_resistance_spawn"

ssG2 = createGroup civilian
"LoBo_Civ_01" createUnit [[7894, 3196, 0], ssG2, "", 0.6, "PRIVATE"]
triSimFrames 90
triScreenshot "I_civilian_spawn"

// These two asserts are the real payload of H/I: if a center is missing,
// createGroup hands back grpNull and the unit never appears.
triAssertGe [(count (units ssG)), 1]
triAssertGe [(count (units ssG2)), 1]

triEndTest
