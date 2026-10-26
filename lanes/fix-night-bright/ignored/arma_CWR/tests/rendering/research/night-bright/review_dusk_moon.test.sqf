// Reviewer check (night-bright): moon key light at dusk, moonlight=1 vs 0, 25 July 1985 (moon ~32 deg up, intensity 1).
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
capJeep = objNull
capDrv = objNull

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

capModeV = {
    capModeName = _this select 0;
    capLit = "OK";
    { capR = triSetLighting _x; if (capR != "OK") then { capLit = capR } } forEach (_this select 1);
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=%3;set=%4", capView, capModeName, capLit, _this select 1] call capLog;
}
capFireOn = { if (not (isNull capFire)) then { capFire inflame true } }

triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }


// ---- dm1900
["dm1900", 6891.9, 8835.4, 3.0, 69.6, -6, [1985, 7, 25, 19, 0], 0, 0, 800] call capBegin
triSimFrames 150
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1900on__remastered"
"dm1900on__remastered" call capRecord
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=0"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1900off__remastered"
"dm1900off__remastered" call capRecord
["classic", ["classic", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1900cl__classic"
"dm1900cl__classic" call capRecord
[] call capClear

// ---- dm1910
["dm1910", 6891.9, 8835.4, 3.0, 69.6, -6, [1985, 7, 25, 19, 10], 0, 0, 800] call capBegin
triSimFrames 150
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1910on__remastered"
"dm1910on__remastered" call capRecord
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=0"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1910off__remastered"
"dm1910off__remastered" call capRecord
["classic", ["classic", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1910cl__classic"
"dm1910cl__classic" call capRecord
[] call capClear

// ---- dm1925
["dm1925", 6891.9, 8835.4, 3.0, 69.6, -6, [1985, 7, 25, 19, 25], 0, 0, 800] call capBegin
triSimFrames 150
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1925on__remastered"
"dm1925on__remastered" call capRecord
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=0"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1925off__remastered"
"dm1925off__remastered" call capRecord
["classic", ["classic", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1925cl__classic"
"dm1925cl__classic" call capRecord
[] call capClear

// ---- dm1940
["dm1940", 6891.9, 8835.4, 3.0, 69.6, -6, [1985, 7, 25, 19, 40], 0, 0, 800] call capBegin
triSimFrames 150
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1940on__remastered"
"dm1940on__remastered" call capRecord
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=0"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1940off__remastered"
"dm1940off__remastered" call capRecord
["classic", ["classic", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1940cl__classic"
"dm1940cl__classic" call capRecord
[] call capClear

// ---- dm1955
["dm1955", 6891.9, 8835.4, 3.0, 69.6, -6, [1985, 7, 25, 19, 55], 0, 0, 800] call capBegin
triSimFrames 150
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1955on__remastered"
"dm1955on__remastered" call capRecord
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=0"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1955off__remastered"
"dm1955off__remastered" call capRecord
["classic", ["classic", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm1955cl__classic"
"dm1955cl__classic" call capRecord
[] call capClear

// ---- dm2015
["dm2015", 6891.9, 8835.4, 3.0, 69.6, -6, [1985, 7, 25, 20, 15], 0, 0, 800] call capBegin
triSimFrames 150
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm2015on__remastered"
"dm2015on__remastered" call capRecord
["remastered", ["remastered", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=0"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm2015off__remastered"
"dm2015off__remastered" call capRecord
["classic", ["classic", "localLights=1", "saturation=1.12", "sky=1", "exposure=1", "bounce=1", "moonlight=1"]] call capModeV
triSimFrames 90
[] call capPin
triSimFrames 3
triScreenshot "dm2015cl__classic"
"dm2015cl__classic" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
