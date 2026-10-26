// look-ref vanilla reference, 1080p look_final views: sinai_lowsun_fp,sinai_desert_noon
// look-director views on Sinai: gen_look_director.py, modes vanilla.
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
// (vanilla: no triCaptureAtPresent; gamma 1.2 is applied offline by the collector)
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


// ---- sinai_lowsun_fp
call { capRain = 0; capEyeAbs = false }
["sinai_lowsun_fp", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin
call { capSimT = 13400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;
    [["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__vanilla"
"sinai_lowsun_fp__vanilla" call capRecord
[] call capClear

// ---- sinai_desert_noon
call { capRain = 0; capEyeAbs = false }
["sinai_desert_noon", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000] call capBegin
call { capSimT = 14000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"vanilla" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__vanilla"
"sinai_desert_noon__vanilla" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
