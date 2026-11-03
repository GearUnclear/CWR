// xp-particles: day smoke stills (the ffh staging by day: side-lit, backlit, front-lit),
// remastered only, for same-exe A/B jobs on the POSEIDON_XP_* switches. Built from the
// gate generator's T1 Abel test (243ef1e0 runner copy): 1920x1080, scale 1.0, 4x MSAA.
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


capBurn = { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }

call { capDetOn = triDeterministic true; format ["view=setup;event=det;deterministic=%1", capDetOn] call capLog }
call { capStepRes = triSetFixedStep (1/60); format ["view=setup;event=step;res=%1", capStepRes] call capLog }
call { capFixedSettle = 300; capSettleWaitMs = 1000; format ["view=setup;event=fixedframes;frames=%1;tail=%2;wait_ms=%3", capFixedSettle, capSettleTail, capSettleWaitMs] call capLog }

// ---- xp_smoke_day_side
call { capRain = 0; capEyeAbs = false }
["xp_smoke_day_side", 7600, 10850, 5, 0, -8, [1985, 7, 12, 9, 0], 0, 0, 1500] call capBegin
call { capSimT = 100; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
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
triWait 1000
[] call capModeTail
triSimFrames 60
[] call capPin
[] call capBurn
call { capRt = triResetTemporal 1; format ["view=%1;event=det;reset=%2", capView, capRt] call capLog }
call { capSd = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSd] call capLog }
triSimFrames 32
triScreenshot "xp_smoke_day_side__gold"
"xp_smoke_day_side__gold" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- xp_smoke_eve_back
call { capRain = 0; capEyeAbs = false }
["xp_smoke_eve_back", 7600, 10850, 5, 290, -4, [1985, 7, 12, 18, 30], 0, 0, 1500] call capBegin
call { capSimT = 100; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
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
triWait 1000
[] call capModeTail
triSimFrames 60
[] call capPin
[] call capBurn
call { capRt = triResetTemporal 1; format ["view=%1;event=det;reset=%2", capView, capRt] call capLog }
call { capSd = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSd] call capLog }
triSimFrames 32
triScreenshot "xp_smoke_eve_back__gold"
"xp_smoke_eve_back__gold" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- xp_smoke_eve_front
call { capRain = 0; capEyeAbs = false }
["xp_smoke_eve_front", 7600, 10850, 5, 110, -6, [1985, 7, 12, 18, 30], 0, 0, 1500] call capBegin
call { capSimT = 100; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
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
triWait 1000
[] call capModeTail
triSimFrames 60
[] call capPin
[] call capBurn
call { capRt = triResetTemporal 1; format ["view=%1;event=det;reset=%2", capView, capRt] call capLog }
call { capSd = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSd] call capLog }
triSimFrames 32
triScreenshot "xp_smoke_eve_front__gold"
"xp_smoke_eve_front__gold" call capRecord
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
