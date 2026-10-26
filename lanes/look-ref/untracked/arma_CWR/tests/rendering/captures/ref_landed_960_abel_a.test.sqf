// look-ref landed reference, 960x640 abel_lighting views (remastered block only): lowsun_fp,sunrise_ridge,noon_fields,forest_edge_golden
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
//  1. lowsun_fp - ACCEPTANCE stand-in for the owner's LightingLowSun.Demo
//  (that mission was on the `demo` world, which the Remaster shadow does not
//  have). Open flat ground on north Malden [7300, 10900] (32 m, slope ~1 deg,
//  nothing within 20 m), first-person eye height, clear sky, 08:00 on 1 March:
//  sun 12.7 deg at az 117.5, 35 deg left of the 152.5 view heading. A Jeep, an
//  M113, a standing SoldierWB and the camouflage-net stand-in are set a few
//  metres apart, turned so each shows a sunlit face (normal ~az 45) and a
//  shaded face (normal ~az 315) to the camera. Remaster vanilla ships no
//  camouflage-net class (CONFIG.BIN, AddOns and DTA searched); the candidate
//  list takes @LoBo's LoBo_camnet when that mod is mounted and otherwise the
//  vanilla Soviet tent CampEast (cloth, half-lit material).
// ----------------------------------------------------------------------------
["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__remastered"
"lowsun_fp__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  2. sunrise_ridge - south-west Malden [4400, 3300] (52 m), looking ENE (60)
//  at the shoulder of the central massif 2 km away (ridge line 4.2 deg at the
//  sun's azimuth). 05:08 on 5 July: sun 5.1 deg at az 64, just clear of the
//  ridge and 4 deg right of centre. Long view distance so the ridge is drawn.
// ----------------------------------------------------------------------------
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__remastered"
"sunrise_ridge__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  3. noon_fields - open upland on north Malden [6250, 10800] (115 m, slope
//  ~2 deg, nothing within 120 m), looking SSE (150) over fields with tree
//  lines at 150-800 m. 12:30 on 5 July: sun 71.9 deg at az 202. A Jeep and a
//  soldier in the foreground for short noon shadows and top-lit shading.
// ----------------------------------------------------------------------------
["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__remastered"
"noon_fields__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  4. forest_edge_golden - north-west coast [4674, 10156] (~45 m): open ground
//  with the sea to the west (no horizon toward the sun) and a wooded slope
//  rising 40-500 m to the east. Looking ENE (75) at the tree line, 18:15 on
//  5 July: sun 12.0 deg at az 290, behind the camera's left shoulder, so the
//  crowns facing the sea are sunlit and the far sides are in shade. A soldier
//  facing the sun stands at 12 m. (Run 1 stood 25 m further west, in a hollow
//  that the Remastered horizon map put in terrain shadow; moved onto the crest.)
// ----------------------------------------------------------------------------
["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__remastered"
"forest_edge_golden__remastered" call capRecord
[] call capClear
call { "view=done;event=done" call capLog }
triEndTest
