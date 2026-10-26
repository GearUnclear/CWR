triSimUntil { alive player }
setViewDistance 1500
0 setOvercast 0.05
0 setFog 0
setDate [1985, 6, 15, 12, 0]
showCinemaBorder false
player setPos [3892, 7095, 0]
player setCaptive true
ssCam = "camera" camCreate [3892, 7095, 10]
ssCam cameraEffect ["internal", "back"]
triSimFrames 10
ssAim = {
 ssT = getPosASL ssP;
 ssCx = (ssT select 0) + ssDist;
 ssCy = (ssT select 2) + ssHigh;
 ssCz = (ssT select 1) - ssDist;
 triSetView [ssCx, ssCy, ssCz, (ssT select 0) - ssCx, ((ssT select 2) + ssAimH) - ssCy, (ssT select 1) - ssCz]
}
ssP = "HeavyReammoBoxRes" createVehicle [3920, 7095, 0]
triAssert [not (isNull ssP)]
ssP setDir 180
ssDist = 2.8
ssHigh = 1.5
ssAimH = 0.2
call ssAim
triSimFrames 20
triScreenshot "01_cache_single_crate"
deleteVehicle ssP
triSimFrames 3
ssP = "ReammoBoxGuer" createVehicle [3920, 7095, 0]
triAssert [not (isNull ssP)]
ssP setDir 180
ssDist = 3.6
ssHigh = 1.8
ssAimH = 0.5
call ssAim
triSimFrames 20
triScreenshot "02_cache_crate_stack"
deleteVehicle ssP
triSimFrames 3
ssP = "Tablemap" createVehicle [3920, 7095, 0]
triAssert [not (isNull ssP)]
ssP setDir 180
ssDist = 2.8
ssHigh = 1.9
ssAimH = 0.7
call ssAim
triSimFrames 20
triScreenshot "03_hq_map_desk"
deleteVehicle ssP
triSimFrames 3
ssP = "ACamp" createVehicle [3920, 7095, 0]
triAssert [not (isNull ssP)]
ssP setDir 180
ssDist = 4
ssHigh = 2
ssAimH = 0.5
call ssAim
triSimFrames 20
triScreenshot "04_hq_small_tent"
deleteVehicle ssP
triSimFrames 3
ssP = "CampEmpty" createVehicle [3920, 7095, 0]
triAssert [not (isNull ssP)]
ssP setDir 180
ssDist = 9
ssHigh = 4.5
ssAimH = 1.3
call ssAim
triSimFrames 20
triScreenshot "05_hq_open_tent"
deleteVehicle ssP
triSimFrames 3
ssP = "CampEast" createVehicle [3920, 7095, 0]
triAssert [not (isNull ssP)]
ssP setDir 180
ssDist = 12
ssHigh = 6
ssAimH = 1.6
call ssAim
triSimFrames 20
triScreenshot "06_hq_large_tent"
deleteVehicle ssP
triSimFrames 3
ssP = "CampEastC" createVehicle [3920, 7095, 0]
triAssert [not (isNull ssP)]
ssP setDir 180
ssDist = 10
ssHigh = 5
ssAimH = 1.5
call ssAim
triSimFrames 20
triScreenshot "07_hq_covered_tent"
deleteVehicle ssP
triSimFrames 3
ssP = "FlagCarrier" createVehicle [3920, 7095, 0]
ssP setFlagTexture "\data\usa_vlajka.pac"
triAssert [not (isNull ssP)]
ssP setDir 180
ssDist = 9
ssHigh = 6
ssAimH = 3
call ssAim
triSimFrames 20
triScreenshot "08_hq_flagpole"
deleteVehicle ssP
triSimFrames 3

triEndTest
