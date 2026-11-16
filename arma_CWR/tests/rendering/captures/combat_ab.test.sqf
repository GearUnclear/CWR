// Combat light A/B (B4) on Abel: gen_combat_ab.py; P-mode stills classic / off / on.
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
#include "capture_lib.sqf"
#include "video/capture_motion.sqf"

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
capProbe = "HeliHEmpty" createVehicle [7600, 10850, 0]
capCam = "camera" camCreate [7600, 10850, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// [forward, right, height, [vx, vy, vz], ammo] call capShellAt: a shell of that
// ammo fired from a point relative to the staging view (createShell).
capShellAt = {
    private ["_p"];
    _p = [_this select 0, _this select 1] call capRel;
    capShellObj = createShell [_this select 4, [_p select 0, _p select 1, _this select 2], _this select 3, player, objNull];
    format ["view=%1;event=shell;ammo=%2;pos=%3;object=%4", capView, _this select 4, _p, capShellObj] call capLog;
}
capPause = { capPauseRes = triPauseGame }
capUnpause = { capPauseRes = triUnpauseGame }
capFixStep = { capFixRes = triSetFixedStep _this }
capBurn = { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
capVolley = { { _x fire (primaryWeapon _x) } forEach capShooters }
capTimerReset = { capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true; capPerfRes = triPerfReset 0 }


// ---- combat_fires and combat_shell (the ffh stage, 5 m up)
call { capRain = 0; capEyeAbs = false }
["combat_fires", 7600, 10850, 5, 0, -12, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    { [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], [12.5, -2.5]];
    { [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], [18, 6], [15, 8.5]];
    capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered" call capMode
[] call capPin
[] call capBurn
0.0333333 call capFixStep
triSimFrames 30
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_fires__classic"
"combat_fires__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_fires__off"
"combat_fires__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_fires__on"
"combat_fires__on" call capRecord
"remastered,combatSmoke=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_fires__smoke0"
"combat_fires__smoke0" call capRecord
"remastered,combatScatter=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_fires__scatter0"
"combat_fires__scatter0" call capRecord
[] call capUnpause
[27, 1, 40, [0, 0, -250], "Shell120"] call capShellAt
triSimFrames 5
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k05__classic"
"combat_shell_k05__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k05__off"
"combat_shell_k05__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k05__on"
"combat_shell_k05__on" call capRecord
[] call capUnpause
triSimFrames 1
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k06__classic"
"combat_shell_k06__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k06__off"
"combat_shell_k06__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k06__on"
"combat_shell_k06__on" call capRecord
[] call capUnpause
triSimFrames 2
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k08__classic"
"combat_shell_k08__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k08__off"
"combat_shell_k08__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k08__on"
"combat_shell_k08__on" call capRecord
[] call capUnpause
triSimFrames 3
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k11__classic"
"combat_shell_k11__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k11__off"
"combat_shell_k11__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k11__on"
"combat_shell_k11__on" call capRecord
[] call capUnpause
triSimFrames 5
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k16__classic"
"combat_shell_k16__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k16__off"
"combat_shell_k16__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_shell_k16__on"
"combat_shell_k16__on" call capRecord
[] call capUnpause
triSimFrames 60
[] call capClear
0 call capFixStep
call { capDriftRes = triPinCloudDrift [] }

// ---- combat_eye (the ffn view: eye height)
call { capRain = 0; capEyeAbs = false }
["combat_eye", 7600, 10850, 1.8, 0, -2, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
call { capSimT = 2600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    { [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], [12.5, -2.5]];
    { [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], [18, 6], [15, 8.5]];
    capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered" call capMode
[] call capPin
[] call capBurn
0.0333333 call capFixStep
triSimFrames 30
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k00__classic"
"combat_eye_k00__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k00__off"
"combat_eye_k00__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k00__on"
"combat_eye_k00__on" call capRecord
"remastered,combatSmoke=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k00__smoke0"
"combat_eye_k00__smoke0" call capRecord
"remastered,combatScatter=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k00__scatter0"
"combat_eye_k00__scatter0" call capRecord
[] call capUnpause
[40, -6, 40, [0, 0, -250], "Shell120"] call capShellAt
triSimFrames 5
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k05__classic"
"combat_eye_k05__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k05__off"
"combat_eye_k05__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k05__on"
"combat_eye_k05__on" call capRecord
[] call capUnpause
triSimFrames 1
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k06__classic"
"combat_eye_k06__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k06__off"
"combat_eye_k06__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k06__on"
"combat_eye_k06__on" call capRecord
[] call capUnpause
triSimFrames 2
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k08__classic"
"combat_eye_k08__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k08__off"
"combat_eye_k08__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k08__on"
"combat_eye_k08__on" call capRecord
[] call capUnpause
triSimFrames 3
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k11__classic"
"combat_eye_k11__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k11__off"
"combat_eye_k11__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_eye_k11__on"
"combat_eye_k11__on" call capRecord
[] call capUnpause
triSimFrames 60
[] call capClear
0 call capFixStep
call { capDriftRes = triPinCloudDrift [] }

// ---- combat_flare (the flr view)
call { capRain = 0; capEyeAbs = false }
["combat_flare", 7600, 10850, 5, 0, -12, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
call { capSimT = 3200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    { [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], [12.5, -2.5]];
    { [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], [18, 6], [15, 8.5]];
    capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered" call capMode
[] call capPin
[] call capBurn
0.0333333 call capFixStep
triSimFrames 30
[30, -35, 45, [0, 0, 12], "Flare"] call capShellAt
triSimFrames 90
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_flare_k90__classic"
"combat_flare_k90__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_flare_k90__off"
"combat_flare_k90__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_flare_k90__on"
"combat_flare_k90__on" call capRecord
[] call capUnpause
triSimFrames 150
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_flare_k240__classic"
"combat_flare_k240__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_flare_k240__off"
"combat_flare_k240__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_flare_k240__on"
"combat_flare_k240__on" call capRecord
[] call capUnpause
triSimFrames 60
[] call capClear
0 call capFixStep
call { capDriftRes = triPinCloudDrift [] }

// ---- combat_muzzle (the research's muz view)
call { capRain = 0; capEyeAbs = false }
["combat_muzzle", 7640, 10860, 1.6, 20, -4, [1985, 7, 12, 23, 0], 0, 0, 800] call capBegin
call { capSimT = 3500; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capShooters = [];
    { capNew = [[_x select 2], _x select 0, _x select 1, -80, "fighter"] call capSpawn; if (not (isNull capNew)) then { capShooters = capShooters + [capNew] } } forEach [[6.5, 1.2, "SoldierWB"], [9.5, -1.8, "SoldierWMG"]];
}
triSimFrames 150
"remastered" call capMode
[] call capPin
[] call capBurn
0.0333333 call capFixStep
triSimFrames 30
[] call capVolley
triSimFrames 4
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k04__classic"
"combat_muzzle_k04__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k04__off"
"combat_muzzle_k04__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k04__on"
"combat_muzzle_k04__on" call capRecord
[] call capUnpause
triSimFrames 1
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k05__classic"
"combat_muzzle_k05__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k05__off"
"combat_muzzle_k05__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k05__on"
"combat_muzzle_k05__on" call capRecord
[] call capUnpause
triSimFrames 1
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k06__classic"
"combat_muzzle_k06__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k06__off"
"combat_muzzle_k06__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k06__on"
"combat_muzzle_k06__on" call capRecord
[] call capUnpause
triSimFrames 2
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k08__classic"
"combat_muzzle_k08__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k08__off"
"combat_muzzle_k08__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_muzzle_k08__on"
"combat_muzzle_k08__on" call capRecord
[] call capUnpause
triSimFrames 60
[] call capClear
0 call capFixStep
call { capDriftRes = triPinCloudDrift [] }

// ---- combat_day (noon)
call { capRain = 0; capEyeAbs = false }
["combat_day", 7600, 10850, 5, 0, -12, [1985, 7, 5, 12, 30], 0, 0, 1000] call capBegin
call { capSimT = 3650; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    { [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], [12.5, -2.5]];
    { [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], [18, 6], [15, 8.5]];
    capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered" call capMode
[] call capPin
[] call capBurn
0.0333333 call capFixStep
triSimFrames 30
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k00__classic"
"combat_day_k00__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k00__off"
"combat_day_k00__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k00__on"
"combat_day_k00__on" call capRecord
[] call capUnpause
[27, 1, 40, [0, 0, -250], "Shell120"] call capShellAt
triSimFrames 5
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k05__classic"
"combat_day_k05__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k05__off"
"combat_day_k05__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k05__on"
"combat_day_k05__on" call capRecord
[] call capUnpause
triSimFrames 1
[] call capPause
"classic" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k06__classic"
"combat_day_k06__classic" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k06__off"
"combat_day_k06__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
triScreenshot "combat_day_k06__on"
"combat_day_k06__on" call capRecord
[] call capUnpause
triSimFrames 60
[] call capClear
0 call capFixStep
call { capDriftRes = triPinCloudDrift [] }

// ---- views without combat light (off must equal on)
call { capRain = 0; capEyeAbs = false }
["nc_sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
call { capSimT = 3800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
triSimFrames 150
"remastered,combatLights=0,combatShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "nc_sunrise_ridge__off"
"nc_sunrise_ridge__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "nc_sunrise_ridge__on"
"nc_sunrise_ridge__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { capRain = 0; capEyeAbs = false }
["nc_road_night", 6895.2, 8836.6, 1.5, 69.6, -2, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin
call { capSimT = 4400; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capJeep = [["Jeep"], -3.4, 0, 0, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;
}
triSimFrames 150
"remastered,combatLights=0,combatShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "nc_road_night__off"
"nc_road_night__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "nc_road_night__on"
"nc_road_night__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

// ---- timing: three interleaved off / on rounds at combat_fires, unpaused
call { capRain = 0; capEyeAbs = false }
["combat_timing", 7600, 10850, 5, 0, -12, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
call { capSimT = 5000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    { [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], [12.5, -2.5]];
    { [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], [18, 6], [15, 8.5]];
    capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered,combatLights=0,combatShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "combat_timing_r1__off"
"combat_timing_r1__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "combat_timing_r1__on"
"combat_timing_r1__on" call capRecord
"remastered,combatLights=0,combatShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "combat_timing_r2__off"
"combat_timing_r2__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "combat_timing_r2__on"
"combat_timing_r2__on" call capRecord
"remastered,combatLights=0,combatShadows=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "combat_timing_r3__off"
"combat_timing_r3__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
triSimFrames 3
triScreenshot "combat_timing_r3__on"
"combat_timing_r3__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- timing at the paused flash frame (combat_eye k05)
call { capRain = 0; capEyeAbs = false }
["combat_flashtiming", 7600, 10850, 1.8, 0, -2, [1985, 7, 12, 23, 0], 0, 0, 1000] call capBegin
call { capSimT = 5600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    { [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], [12.5, -2.5]];
    { [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], [18, 6], [15, 8.5]];
    capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"remastered" call capMode
[] call capPin
[] call capBurn
0.0333333 call capFixStep
triSimFrames 30
[40, -6, 40, [0, 0, -250], "Shell120"] call capShellAt
triSimFrames 5
[] call capPause
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
[] call capTimerReset
triSimFrames 60
[] call capReadTimers
triScreenshot "combat_flashtiming_r1__off"
"combat_flashtiming_r1__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
[] call capTimerReset
triSimFrames 60
[] call capReadTimers
triScreenshot "combat_flashtiming_r1__on"
"combat_flashtiming_r1__on" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
[] call capTimerReset
triSimFrames 60
[] call capReadTimers
triScreenshot "combat_flashtiming_r2__off"
"combat_flashtiming_r2__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
[] call capTimerReset
triSimFrames 60
[] call capReadTimers
triScreenshot "combat_flashtiming_r2__on"
"combat_flashtiming_r2__on" call capRecord
"remastered,combatLights=0,combatShadows=0" call capModeQuick
triSimFrames 6
[] call capTimerReset
triSimFrames 60
[] call capReadTimers
triScreenshot "combat_flashtiming_r3__off"
"combat_flashtiming_r3__off" call capRecord
"remastered" call capModeQuick
triSimFrames 6
[] call capTimerReset
triSimFrames 60
[] call capReadTimers
triScreenshot "combat_flashtiming_r3__on"
"combat_flashtiming_r3__on" call capRecord
[] call capUnpause
[] call capClear
0 call capFixStep
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
