// look-ref vanilla reference, look_director_optics with one mode
// look director: HUD and a rifle optic in morning mist, Original and Overhaul (hand-written)
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Eden"
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
capFire = objNull
capBurnV = objNull
capJeep = objNull
capDrv = objNull

call { format ["view=setup;event=window_boot;size=%1", triGetWindowSize] call capLog }
// (vanilla: no triCaptureAtPresent; gamma 1.2 is applied offline by the collector)
triSetMsaa 4
triSetRenderScale 1.0
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capGrpE = createGroup east
capProbe = "HeliHEmpty" createVehicle [9200, 3700, 0]
capCam = "camera" camCreate [9200, 3700, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Eden;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// ---- optics: the player on foot (HUD), then the rifle's optics, frames 0..3
// after the switch, with the fog and with volFog=0.
call { capRain = 0; capEyeAbs = false }
["optics", 9200, 3700, 1.75, 288, -2, [1985, 7, 5, 9, 30], 0.7, 0.3, 1500] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT }
capUseView = false
player setPos [9200, 3700, 0]
player setDir 288
player addMagazine "M21"
player addMagazine "M21"
player addWeapon "M21"
player selectWeapon "M21"
capCam cameraEffect ["terminate", "back"]
camDestroy capCam
triSimFrames 60
capOptIdx = 0
{
    _x call capMode;
    player switchCamera "INTERNAL";
    triSimFrames 60;
    // The same HUD state in both modes (look director round 2): the weapon
    // box shows for a few seconds after a weapon change, so each mode
    // re-arms the rifle 30 frames before its HUD shot.
    removeAllWeapons player;
    player addMagazine "M16";
    player addMagazine "M16";
    player addWeapon "M16";
    player selectWeapon "M16";
    triSimFrames 30;
    [] call capPin;
    triSimFrames 3;
    triScreenshot (format ["optics__hud_%1", capOptIdx]);
    (format ["optics__hud_%1", capOptIdx]) call capRecord;
    player switchCamera "GUNNER";
    triSimFrames 1;
    triScreenshot (format ["optics__gun_%1_f1", capOptIdx]);
    triSimFrames 1;
    triScreenshot (format ["optics__gun_%1_f2", capOptIdx]);
    triSimFrames 30;
    triScreenshot (format ["optics__gun_%1_f30", capOptIdx]);
    (format ["optics__gun_%1_f30", capOptIdx]) call capRecord;
    // (A screenshot is taken at the next present: without these frames the
    // next mode's switch landed before it, look director round 2.)
    triSimFrames 3;
    capOptIdx = capOptIdx + 1;
} forEach ["vanilla"]
call { format ["view=%1;event=camera;cam=%2", capView, cameraView] call capLog }
call { "view=done;event=done" call capLog }
triEndTest
