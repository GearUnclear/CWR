// ============================================================================
//  Research-only water views (Classic | Remastered) on Abel's west coast.
//  Same rig and per-view block as tests/rendering/captures/abel_lighting.
//  Coast at [1550, 5150]: beach ~3.5 m ASL, waterline ~20 m west, open sea
//  along heading 278; the ground rises ~9 m per 50 m inland (heightmap).
//  Sun numbers from captures/sunplan.py for 1985-07-05.
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

// ----------------------------------------------------------------------------
//  coastline_sunset: as in abel_lighting (sun 8.4 deg, az 293, 15 deg right).
// ----------------------------------------------------------------------------
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__classic"
"coastline_sunset__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__remastered"
"coastline_sunset__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  sea_noon: eye 12 m over the beach (bluff stand-in), looking W and down 12 deg; sun 71.9 deg at az 202, 76 deg left and behind.
// ----------------------------------------------------------------------------
["sea_noon", 1550, 5150, 12, 278, -12, [1985, 7, 5, 12, 30], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_noon__classic"
"sea_noon__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_noon__remastered"
"sea_noon__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  sea_away_from_sun: looking W with the sun behind (48.4 deg at az 100); open sea, no glint.
// ----------------------------------------------------------------------------
["sea_away_from_sun", 1550, 5150, 3, 278, -5, [1985, 7, 5, 9, 0], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_away_from_sun__classic"
"sea_away_from_sun__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sea_away_from_sun__remastered"
"sea_away_from_sun__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  coast_shore_midsun: eye 15 m over the beach, looking W and down 22 deg; sun 31.6 deg at az 275 ahead, so its glint lies ~24 m out, just past the surf.
// ----------------------------------------------------------------------------
["coast_shore_midsun", 1550, 5150, 15, 278, -22, [1985, 7, 5, 16, 30], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_shore_midsun__classic"
"coast_shore_midsun__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coast_shore_midsun__remastered"
"coast_shore_midsun__remastered" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
