// RESEARCH (temporal track) smoke: does TAA run, and what does it do to a still?
#include "capture_lib.sqf"
#include "temporal_lib.sqf"

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
capPanYaw = 0
capPanSide = 0
capPanDYaw = 0
capPanDSide = 0

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
call { "view=setup;event=setup;world=Abel;research=temporal_smoke" call capLog }

["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
}
triSimFrames 150
["remastered", "remastered", 4, 1.5, []] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__remastered"
"forest_edge_golden__remastered" call capRecord
["taa15", "remastered", 0, 1.5, ["taa=1"]] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 30
triScreenshot "forest_edge_golden__taa15"
"forest_edge_golden__taa15" call capRecord
["noaa10", "remastered", 0, 1.0, []] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__noaa10"
"forest_edge_golden__noaa10" call capRecord
["taa10", "remastered", 0, 1.0, ["taa=1"]] call capVariant
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 30
triScreenshot "forest_edge_golden__taa10"
"forest_edge_golden__taa10" call capRecord
// a short yaw pan under TAA: motion vectors debug, then the image
["taa15pan", "remastered", 0, 1.5, ["taa=1"]] call capVariant
triSimFrames 60
call { capPanDYaw = 0.1; capPanDSide = 0; [-4, 0] call capPanReset }
call { 40 call capPanRun }
triScreenshot "forest_edge_golden__taa15pan"
"forest_edge_golden__taa15pan" call capRecord
call { capKnobRes = triSetLighting "debug=5"; 2 call capPanRun }
triScreenshot "forest_edge_golden__taa15motion"
"forest_edge_golden__taa15motion" call capRecord
["msaa15pan", "remastered", 4, 1.5, []] call capVariant
triSimFrames 60
call { capPanDYaw = 0.1; capPanDSide = 0; [-4, 0] call capPanReset }
call { 40 call capPanRun }
triScreenshot "forest_edge_golden__msaa15pan"
"forest_edge_golden__msaa15pan" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
