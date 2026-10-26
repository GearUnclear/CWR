// crown-nan research capture (generated; research only, git-excluded)
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
capDbg = "none"

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
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present;run=crown-nan" call capLog }

// ---- forest_edge_golden
["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call { [["SoldierWB"], 12, 2, 215, "man"] call capSpawn; }
triSimFrames 150
"remastered" call capMode
call { capDbg = triSetLighting "debug=0" }
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__remastered"
"forest_edge_golden__remastered" call capRecord
call { capDbg = triSetLighting "debug=3" }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__dbg3"
"forest_edge_golden__dbg3" call capRecord
call { capDbg = triSetLighting "debug=0" }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__late"
"forest_edge_golden__late" call capRecord
[] call capClear

// ---- woods_mixed_noon
["woods_mixed_noon", 2864, 6656, 1.75, 300, -5, [1985, 7, 5, 14, 0], 0, 0, 1500] call capBegin
call { [["SoldierWB"], 7, 1, 160, "man"] call capSpawn; }
triSimFrames 150
"remastered" call capMode
call { capDbg = triSetLighting "debug=0" }
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_mixed_noon__remastered"
"woods_mixed_noon__remastered" call capRecord
call { capDbg = triSetLighting "debug=3" }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "woods_mixed_noon__dbg3"
"woods_mixed_noon__dbg3" call capRecord
call { capDbg = triSetLighting "debug=0" }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "woods_mixed_noon__late"
"woods_mixed_noon__late" call capRecord
[] call capClear

// ---- woods_pine2_evening
["woods_pine2_evening", 6940, 9964, 1.75, 240, -5, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { [["SoldierWB"], 8, -1, 200, "man"] call capSpawn; }
triSimFrames 150
"remastered" call capMode
call { capDbg = triSetLighting "debug=0" }
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening__remastered"
"woods_pine2_evening__remastered" call capRecord
call { capDbg = triSetLighting "debug=3" }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening__dbg3"
"woods_pine2_evening__dbg3" call capRecord
call { capDbg = triSetLighting "debug=0" }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "woods_pine2_evening__late"
"woods_pine2_evening__late" call capRecord
[] call capClear

// ---- town_day
["town_day", 7152, 8915, 1.75, 345, 3, [1985, 7, 5, 16, 30], 0, 0, 1200] call capBegin
call { [["Jeep"], 14, 4, 80, "veh"] call capSpawn; [["SoldierWB"], 9, -1, 170, "man"] call capSpawn; }
triSimFrames 150
"remastered" call capMode
call { capDbg = triSetLighting "debug=0" }
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_day__remastered"
"town_day__remastered" call capRecord
call { capDbg = triSetLighting "debug=3" }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "town_day__dbg3"
"town_day__dbg3" call capRecord
call { capDbg = triSetLighting "debug=0" }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "town_day__late"
"town_day__late" call capRecord
[] call capClear

triEndTest
