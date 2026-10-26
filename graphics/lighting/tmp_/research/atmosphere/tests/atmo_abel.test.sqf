// Atmosphere research track, Abel views: Classic | Remastered | Remastered + atmosphere.
// Copy of the committed capture rig (capture_lib.sqf) with the prototype's knobs.
#include "capture_lib.sqf"
#include "atmo_lib.sqf"

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
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

// ---------------------------------------------------------------- sunrise_ridge
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__classic"
"sunrise_ridge__classic" call capRecord
["remastered", []] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__remastered"
"sunrise_ridge__remastered" call capRecord
["atmo", ["atmosphere=1", "atmoFog=1", "atmoLight=1"]] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__atmo"
"sunrise_ridge__atmo" call capRecord
["dbgdir", ["atmosphere=1", "atmoDebug=3"]] call capVariant
triSimFrames 10
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__dbgdir"
"sunrise_ridge__dbgdir" call capRecord
["dbgatmo", ["atmosphere=1", "atmoDebug=2"]] call capVariant
triSimFrames 10
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__dbgatmo"
"sunrise_ridge__dbgatmo" call capRecord
[] call capClear

// ---------------------------------------------------------------- noon_fields
["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__classic"
"noon_fields__classic" call capRecord
["remastered", []] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__remastered"
"noon_fields__remastered" call capRecord
["atmo", ["atmosphere=1", "atmoFog=1", "atmoLight=1"]] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__atmo"
"noon_fields__atmo" call capRecord
["dbgcloud", ["atmosphere=1", "atmoDebug=1"]] call capVariant
triSimFrames 10
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__dbgcloud"
"noon_fields__dbgcloud" call capRecord
[] call capClear

// ---------------------------------------------------------------- coastline_sunset
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__classic"
"coastline_sunset__classic" call capRecord
["remastered", []] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__remastered"
"coastline_sunset__remastered" call capRecord
["atmo", ["atmosphere=1", "atmoFog=1", "atmoLight=1"]] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__atmo"
"coastline_sunset__atmo" call capRecord
[] call capClear

// ---------------------------------------------------------------- forest_edge_golden
["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__classic"
"forest_edge_golden__classic" call capRecord
["remastered", []] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__remastered"
"forest_edge_golden__remastered" call capRecord
["atmo", ["atmosphere=1", "atmoFog=1", "atmoLight=1"]] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__atmo"
"forest_edge_golden__atmo" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
