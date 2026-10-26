// SSR research captures, Abel. Three-way: classic | remastered | remastered + SSR,
// plus debug shots of the surface tags. Research copy (gfx-ssr), not committed.
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
triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present;track=ssr" call capLog }

// ---------------------------------------------------------------------------
// bay_morning: east-coast shore [9500, 7100] (3.5 m) looking WSW (240) across
// ~2 km of calm bay to the hills of the main island (4.9 deg above the eye).
// 06:30 on 5 July: sun 19.9 deg at az 76, behind the camera: the far hills
// are front lit. A patrol boat 70 m out.
// ---------------------------------------------------------------------------
["bay_morning", 9500, 7100, 2, 240, -2, [1985, 7, 5, 6, 30], 0, 0, 4000] call capBegin
call {
    [["BoatW", "BoatE", "Boat"], 70, 12, 60, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_morning__classic"
"bay_morning__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_morning__remastered"
"bay_morning__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_morning__ssr"
"bay_morning__ssr" call capRecord
["ssrcalm", ["ssr=1", "ssrDebug=0", "ssrWave=0.35"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_morning__ssrcalm"
"bay_morning__ssrcalm" call capRecord
call { capKnob = triSetLighting "ssrWave=1" }
["ssrtags", ["ssr=1", "ssrDebug=1"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_morning__ssrtags"
"bay_morning__ssrtags" call capRecord
["ssrhits", ["ssr=1", "ssrDebug=3"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_morning__ssrhits"
"bay_morning__ssrhits" call capRecord
call { capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssr=0" }
[] call capClear

// ---------------------------------------------------------------------------
// coastline_sunset: as the committed set (west coast, sun 8.4 deg ahead right).
// ---------------------------------------------------------------------------
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__classic"
"coastline_sunset__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__remastered"
"coastline_sunset__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__ssr"
"coastline_sunset__ssr" call capRecord
["ssrcalm", ["ssr=1", "ssrDebug=0", "ssrWave=0.35"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__ssrcalm"
"coastline_sunset__ssrcalm" call capRecord
call { capKnob = triSetLighting "ssrWave=1" }
["ssrhits", ["ssr=1", "ssrDebug=3"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__ssrhits"
"coastline_sunset__ssrhits" call capRecord
call { capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssr=0" }
[] call capClear

// ---------------------------------------------------------------------------
// lowsun_fp: the acceptance stand-in (paint and glass on the Jeep and M113).
// ---------------------------------------------------------------------------
["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__classic"
"lowsun_fp__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__remastered"
"lowsun_fp__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssr"
"lowsun_fp__ssr" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssrtags"
"lowsun_fp__ssrtags" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=0.8"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssrwet"
"lowsun_fp__ssrwet" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssr=0" }
[] call capClear

// ---------------------------------------------------------------------------
// town_night: Saint Louis street lamps and a campfire; dry SSR, then the same
// street wet (ssrWet 0.8: after rain, no falling rain so the moon stays).
// ---------------------------------------------------------------------------
["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__classic"
"town_night__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__remastered"
"town_night__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__ssr"
"town_night__ssr" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=0.8"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__ssrwet"
"town_night__ssrwet" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWet=0.8"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_night__ssrtags"
"town_night__ssrtags" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssr=0" }
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
