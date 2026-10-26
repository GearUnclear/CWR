// round 3 overhaul-only copy of eden_lighting (remastered block only)
// ============================================================================
//  Remastered lighting: Classic | Remastered capture pairs on Eden (Everon),
//  the cloud views. Same rig and per-mode block as abel_lighting.test.sqf;
//  read that file's header and capture_lib.sqf first.
//
//  Both views share one spot: the south-east Everon highland [9200, 3700]
//  (271 m), looking W (288) down over the lowland 60-150 m below, with the
//  town of Levie [7554, 4737] 1.9 km away near the centre of the frame and
//  4 km of land in view (the ridge north of the view line rises just above eye level at 315-330). 14:30 on 5 July: sun 54.3 deg at az 253, 35 deg left
//  of the view, so the landscape is side-lit and cloud shadows (Remastered:
//  drifting shadows whose mean equals the classic uniform dimming) land on
//  sunlit ground. The sim clock is re-pinned before each shot so the cloud
//  field is in the same place for Classic and Remastered.
// ============================================================================

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

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [9200, 3700, 0]
capCam = "camera" camCreate [9200, 3700, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Eden;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

// ----------------------------------------------------------------------------
//  1. overcast_broken - overcast 0.5: broken cloud. A Jeep at 12 m is the
//  near reference for how a cloud shadow and the sun treat a vehicle.
// ----------------------------------------------------------------------------
["overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_broken__remastered"
"overcast_broken__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  2. overcast_heavy - overcast 0.9, rain held at 0: the flat-light case,
//  where the sun is almost fully dimmed and the sky fill does the work.
// ----------------------------------------------------------------------------
["overcast_heavy", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.9, 0, 3500] call capBegin
call {
    [["Jeep"], 12, -5, 70, "veh"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "overcast_heavy__remastered"
"overcast_heavy__remastered" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
