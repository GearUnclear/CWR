// look-ref landed reference, 960x640 abel_lighting views (remastered block only): tank_gunner_interior
// ============================================================================
//  Remastered lighting: Classic | Remastered capture pairs on Abel (Malden).
//  Not a regression test: a repeatable photo run, driven by run-captures.ps1
//  (see README.md beside this file for the view table and how to run it).
//
//  Every matched view runs the same block twice, Classic then Remastered:
//    capMode                            pin the object detail, switch the lighting
//                                       model (every knob at its default), wait
//                                       until the view has loaded, restart the
//                                       GPU / CPU timers
//    triSimFrames 90                    enough frames for the smoothed timers
//    capReadTimers                      keep the timings of this mode
//    capPin                             same date, weather, sim clock, props, view
//    triSimFrames 3
//    triScreenshot "<view>__<mode>"     the presented frame (triCaptureAtPresent)
//    capRecord                          one UDCAP;event=shot log line
//  The remastered exposure meter is a pure function of the sun height
//  (LightRig.cpp), with no temporal adaptation, so nothing has to "settle" in
//  the tonemap between the two shots.
// ============================================================================

#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Abel"
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
capView = "setup"
capModeName = "none"
capLit = "none"
capGpu = "none"
capPerf = "none"

// Capture settings shared by every view: 4x MSAA, 150% render scale (both
// applied at the next frame boundary), and the presented, post-gamma frame.
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]

// The horizon map (TerrainHorizon) is built off-thread when the island loads;
// give it time before the first frame that matters.
triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

// ----------------------------------------------------------------------------
//  8-9. tank gunner: interior and optics. The player is put in the gunner seat
//  of an M60 at the view-1 spot [7300, 10900] facing 152.5, with an M113, a
//  Jeep and a soldier parked 50-70 m down the gun line. 10:00 on 5 July: sun
//  59.3 deg at az 114.6. The cutscene camera and triSetView are released here:
//  these are the player's own cameras (HUD included, identical in both modes).
//  INTERNAL is the vehicle interior view (the remastered post pass sees a
//  cleared world depth there, so no AO / shafts); GUNNER is the optic, a
//  screen overlay inside the world pass that turns bloom and shafts off.
//  The `cam=` field of each shot line records which camera was really active.
// ----------------------------------------------------------------------------
["tank_gunner_interior", 7300, 10900, 1.75, 152.5, 0, [1985, 7, 5, 10, 0], 0, 0, 1500] call capBegin
call {
    [["M113"], 70, 4, 90, "veh"] call capSpawn;
    [["Jeep"], 55, -10, -60, "veh"] call capSpawn;
    [["SoldierWB"], 50, 0, 180, "man"] call capSpawn;
}
call {
    capUseView = false;
    capCam cameraEffect ["terminate", "back"];
    capClearRes = triClearView;
    capTankCls = ["M60", "M1Abrams", "T72"] call capPick;
    capTank = objNull;
    if (capTankCls != "") then { capTank = capTankCls createVehicle [7300, 10900, 0] };
    if (not (isNull capTank)) then {
        capTank setDir 152.5;
        capTank setPos [7300, 10900, 0];
        capTank allowDamage false;
        player moveInGunner capTank;
    };
    format ["view=tank_gunner_interior;event=tank;class=%1;inTank=%2", capTankCls, not ((vehicle player) == player)] call capLog;
}
triSimFrames 60
call { (vehicle player) switchCamera "INTERNAL" }
triSimFrames 60
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "tank_gunner_interior__remastered"
"tank_gunner_interior__remastered" call capRecord
call { capView = "tank_gunner_optics"; capSimT = time; (vehicle player) switchCamera "GUNNER" }
triSimFrames 60
// The same HUD state in both modes (look director round 2): the action menu
// and the target box time out a few seconds after the gunner takes his seat,
// and the ammo line's colour follows the gun's reload; each mode puts the
// player back in the seat and waits the same frames before its shot.
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
call { player setPos [7290, 10885, 0] }
triSimFrames 5
call { player moveInGunner capTank }
triSimFrames 400
call { (vehicle player) switchCamera "GUNNER" }
triSimFrames 60
[] call capPin
triSimFrames 3
triScreenshot "tank_gunner_optics__remastered"
"tank_gunner_optics__remastered" call capRecord

call { "view=done;event=done" call capLog }
triEndTest
