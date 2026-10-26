// Research-only (git-excluded): shaft-veil bug fix evidence on Sinai with
// @LoBo. The sinai_lowsun_fp view of sinai_lobo_lighting.test.sqf, shot
// Classic, Remastered, and Remastered with the sun shafts off.
#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Sinai"
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
capProbe = "HeliHEmpty" createVehicle [8350, 2950, 0]
capCam = "camera" camCreate [8350, 2950, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Sinai;msaa=4;scale=1.5;window=960x640;capture=present;lobo=%1", "LoBo_camnet" call capHas] call capLog }

["sinai_lowsun_fp", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;
    [["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__classic"
"sinai_lowsun_fp__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__remastered"
"sinai_lowsun_fp__remastered" call capRecord
"shafts=0" call capMode
triSimFrames 30
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp_noshafts__remastered"
"sinai_lowsun_fp_noshafts__remastered" call capRecord
"shafts=1" call capMode
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
