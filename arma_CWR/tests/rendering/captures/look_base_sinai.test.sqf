// look-light hero views on Sinai: gen_look_light.py, modes classic, remastered.
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
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
capFire = objNull
capBurnV = objNull
capJeep = objNull
capDrv = objNull

call { format ["view=setup;event=window_boot;size=%1", triGetWindowSize] call capLog }
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.0
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capGrpE = createGroup east
capProbe = "HeliHEmpty" createVehicle [7300, 10300, 0]
capCam = "camera" camCreate [7300, 10300, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Sinai;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// ---- sinai_desert_noon
call { capRain = 0; capEyeAbs = false }
["sinai_desert_noon", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000] call capBegin
call { capSimT = 6200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__classic"
"sinai_desert_noon__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__remastered"
"sinai_desert_noon__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
