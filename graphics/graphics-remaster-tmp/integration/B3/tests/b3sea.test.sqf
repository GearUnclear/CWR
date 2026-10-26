// B3 lane, review fix: the low beams on the B5 sea (seaBody), off and on.
// Written by hand on the b3v3 pattern: 1920x1080 windowed, render scale 1.0,
// 4x MSAA, presented frame. The hero view rides along for off equals before.
//   b3_sea_beam        moonless, a Jeep on the shore facing the sea, the camera
//                      8 m behind it: the beam's pool on the water
//   b3_sea_beam_front  the same Jeep seen from 40 m out over the water: the
//                      beam toward the camera and the lamps' gleam on the sea
// The shore is found at run time: from the b3_sea_moon origin (9240, 6950)
// along heading 110, the first point whose ground is more than 0.8 m above
// the sea; the Jeep stands 2 m further inland.
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
capProbe = "HeliHEmpty" createVehicle [6895.2, 8836.6, 0]
capCam = "camera" camCreate [6895.2, 8836.6, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present", triGetWindowSize] call capLog }

// ---- the shore
call {
    private ["_d", "_g", "_found", "_k", "_prof"];
    _d = 0;
    _found = false;
    while { (not _found) && (_d < 2500) } do {
        _g = [9240 + _d * (sin 110), 6950 + _d * (cos 110)] call capGround;
        if (_g > 0.8) then { _found = true } else { _d = _d + 2 };
    };
    capShoreD = _d + 2;
    capJX = 9240 + capShoreD * (sin 110);
    capJZ = 6950 + capShoreD * (cos 110);
    _prof = [];
    _k = -40;
    while { _k <= 40 } do {
        _prof = _prof + [[_k, [9240 + (capShoreD + _k) * (sin 110), 6950 + (capShoreD + _k) * (cos 110)] call capGround]];
        _k = _k + 5;
    };
    format ["view=probe;event=shore;found=%1;d=%2;jeep=%3;profile=%4", _found, capShoreD, [capJX, capJZ], _prof] call capLog;
}

// ---- night_road_flat_drive_moonless (off = remastered,nightLights=0,moonShadows=0,stars=0; on = remastered)
call { capRain = 0; capEyeAbs = false }
["night_road_flat_drive_moonless", 6895.2, 8836.6, 1.5, 69.6, -2, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin
call { capSimT = 5600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capJeep = [["Jeep"], -3.4, 0, 0, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;
}
triSimFrames 150
"remastered,nightLights=0,moonShadows=0,stars=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "night_road_flat_drive_moonless__off"
"night_road_flat_drive_moonless__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "night_road_flat_drive_moonless__on"
"night_road_flat_drive_moonless__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

// ---- b3_sea_beam (off = remastered,nightLights=0,moonShadows=0,stars=0; on = remastered)
call { capRain = 0; capEyeAbs = false }
call { [ "b3_sea_beam", capJX + 8 * (sin 110), capJZ + 8 * (cos 110), 2.6, 290, -6, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin }
call { capSimT = 6200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capJeep = [["Jeep"], 8, 0, 0, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3;pos=%4", capView, capJeep, driver capJeep, getPosASL capJeep] call capLog;
}
triSimFrames 150
"remastered,nightLights=0,moonShadows=0,stars=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "b3_sea_beam__off"
"b3_sea_beam__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "b3_sea_beam__on"
"b3_sea_beam__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

// ---- b3_sea_beam_front (off = remastered,nightLights=0,moonShadows=0,stars=0; on = remastered)
call { capRain = 0; capEyeAbs = true }
call { [ "b3_sea_beam_front", capJX - 40 * (sin 110), capJZ - 40 * (cos 110), 2.2, 110, -3, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin }
call { capSimT = 6800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    capJeep = [["Jeep"], 40, 0, 180, "veh"] call capSpawn;
    capDrv = objNull;
    if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capJX, capJZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };
    format ["view=%1;event=jeep;jeep=%2;driver=%3;pos=%4", capView, capJeep, driver capJeep, getPosASL capJeep] call capLog;
}
triSimFrames 150
"remastered,nightLights=0,moonShadows=0,stars=0" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "b3_sea_beam_front__off"
"b3_sea_beam_front__off" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "b3_sea_beam_front__on"
"b3_sea_beam_front__on" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }
call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }

call { "view=done;event=done" call capLog }
triEndTest
