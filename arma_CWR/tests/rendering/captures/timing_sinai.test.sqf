// Quiet-GPU timing on Sinai: gen_timing.py, 3 interleaved rounds.
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
capProbe = "HeliHEmpty" createVehicle [8350, 2950, 0]
capCam = "camera" camCreate [8350, 2950, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Sinai;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// ---- sinai_lowsun_fp round 1 (temporal, shade (Sinai clips))
call { capRain = 0; capEyeAbs = false }
["sinai_lowsun_fp_r1", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
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
triScreenshot "sinai_lowsun_fp_r1__classic"
"sinai_lowsun_fp_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp_r1__remastered"
"sinai_lowsun_fp_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sinai_desert_noon round 1 (gate-off reference (Sinai noon))
call { capRain = 0; capEyeAbs = false }
["sinai_desert_noon_r1", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000] call capBegin
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
triScreenshot "sinai_desert_noon_r1__classic"
"sinai_desert_noon_r1__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon_r1__remastered"
"sinai_desert_noon_r1__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sinai_lowsun_fp round 2 (temporal, shade (Sinai clips))
call { capRain = 0; capEyeAbs = false }
["sinai_lowsun_fp_r2", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
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
triScreenshot "sinai_lowsun_fp_r2__classic"
"sinai_lowsun_fp_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp_r2__remastered"
"sinai_lowsun_fp_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sinai_desert_noon round 2 (gate-off reference (Sinai noon))
call { capRain = 0; capEyeAbs = false }
["sinai_desert_noon_r2", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000] call capBegin
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
triScreenshot "sinai_desert_noon_r2__classic"
"sinai_desert_noon_r2__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon_r2__remastered"
"sinai_desert_noon_r2__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sinai_lowsun_fp round 3 (temporal, shade (Sinai clips))
call { capRain = 0; capEyeAbs = false }
["sinai_lowsun_fp_r3", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
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
triScreenshot "sinai_lowsun_fp_r3__classic"
"sinai_lowsun_fp_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp_r3__remastered"
"sinai_lowsun_fp_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- sinai_desert_noon round 3 (gate-off reference (Sinai noon))
call { capRain = 0; capEyeAbs = false }
["sinai_desert_noon_r3", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000] call capBegin
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
triScreenshot "sinai_desert_noon_r3__classic"
"sinai_desert_noon_r3__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon_r3__remastered"
"sinai_desert_noon_r3__remastered" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
