// Research-only (git-excluded): shaft-veil bug fix evidence on Abel. The
// lowsun_fp and sunrise_ridge views of abel_lighting.test.sqf, each shot
// Classic, Remastered, and Remastered with the sun shafts off (reference for
// what a near object looks like with no shaft term at all).
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
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__remastered"
"lowsun_fp__remastered" call capRecord
"shafts=0" call capMode
triSimFrames 30
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp_noshafts__remastered"
"lowsun_fp_noshafts__remastered" call capRecord
"shafts=1" call capMode
[] call capClear

["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__classic"
"sunrise_ridge__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__remastered"
"sunrise_ridge__remastered" call capRecord
"shafts=0" call capMode
triSimFrames 30
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge_noshafts__remastered"
"sunrise_ridge_noshafts__remastered" call capRecord
"shafts=1" call capMode
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
