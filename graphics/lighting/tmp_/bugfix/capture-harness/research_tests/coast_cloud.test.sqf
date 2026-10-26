// Research only (git-excluded): is the shade over the near shore and sea in
// the AFTER run's coastline_sunset Remastered shot a cloud shadow under the
// clear sky? Same view and settings as the capture rig; the sim clock pinned
// to the AFTER run's 6.216 s and the BEFORE run's 6.684 s with cloud shadows
// on, then 6.216 s with cloud shadows off.

#include "../../captures/capture_lib.sqf"

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
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240
call { capLit = triSetLighting "classic"; capModeName = "classic" }


["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
"remastered" call capMode
triSimFrames 90
call { capSimT = 6.216; capPinTime = true; [] call capPin }
triSimFrames 3
triScreenshot "coast_t6216_cloud1"
call { capSimT = 6.684; [] call capPin }
triSimFrames 3
triScreenshot "coast_t6684_cloud1"
call { capLit = triSetLighting "remastered,cloudShadows=0"; capSimT = 6.216; [] call capPin; format ["view=coast;event=knob;lit=%1", capLit] call capLog }
triSimFrames 3
triScreenshot "coast_t6216_cloud0"
call { "view=done;event=done" call capLog }
triEndTest
