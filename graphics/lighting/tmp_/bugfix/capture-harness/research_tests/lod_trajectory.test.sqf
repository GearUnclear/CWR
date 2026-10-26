// Research only (git-excluded): what the capture rig's first shot after a
// camera jump sees, with the object detail adaptive (the stock behaviour, the
// "before") and pinned at its finest (the fix). Same build, same view
// (sunrise_ridge, VD 4000). Every 10 frames for 400 frames after the jump it
// logs the last frame's draws / objects / LOD width (triSceneStats) and the
// window's CPU frame (triPerfStats). A screenshot is taken 30 frames after
// each jump (where the old rig's 150 + 90 frame wait is still inside the
// first ~5 s) and again at the end.

#include "../../captures/capture_lib.sqf"

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
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240
call { capLit = triSetLighting "classic"; capModeName = "classic" }

capTraj = {
    capI = 0;
    while {capI < 40} do {
        capPerfR = triPerfReset 0;
        capSimR = triSimFrames 10;
        format ["view=%1;event=traj;phase=%2;step=%3;%4;perf=%5", capView, capPhase, capI, triSceneStats, triPerfStats 0] call capLog;
        capI = capI + 1;
    };
}

// ---- A: adaptive detail (stock), jump from north Malden to sunrise_ridge
call { capPinR = triPinObjectDetail 0; capPhase = "adaptive" }
["away", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
triSimFrames 120
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 30
triScreenshot "adaptive_f30"
call { format ["view=sunrise_ridge;event=shotA;%1", triSceneStats] call capLog }
call { [] call capTraj }
triScreenshot "adaptive_f430"
call { format ["view=sunrise_ridge;event=shotA_end;%1", triSceneStats] call capLog }

// ---- B: pinned at the finest detail, the same jump
call { capPinR = triPinObjectDetail 1; capPhase = "pinned" }
["away", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
triSimFrames 120
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 30
triScreenshot "pinned_f30"
call { format ["view=sunrise_ridge;event=shotB;%1", triSceneStats] call capLog }
call { [] call capTraj }
triScreenshot "pinned_f430"
call { format ["view=sunrise_ridge;event=shotB_end;%1", triSceneStats] call capLog }

// ---- C: pinned, the same jump, then the capture rig's load wait
["away", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
triSimFrames 120
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capS = triSettleScene [300, 30, 2]; format ["view=sunrise_ridge;event=settle_probe;settle=%1;%2", capS, triSceneStats] call capLog }

call { "view=done;event=done" call capLog }
triEndTest
