triSimUntil { gmZoneCount > 0 }
player setCaptive true
0 setOvercast 0.05
0 setFog 0
setDate [1985, 6, 15, 12, 0]
showCinemaBorder false
ssCam = "camera" camCreate [7100, 6070, 10]
ssCam cameraEffect ["internal", "back"]
triAssert [gmHqEstablish ((gmZone (gmZoneIndex "Village")) select 8)]
triAssert [gmHqIndoors]
ssA = getPosASL gmHqProp
ssB = getPosASL gmHqCache
ssX = ((ssA select 0) + (ssB select 0)) / 2
ssZ = ((ssA select 1) + (ssB select 1)) / 2
ssY = ssA select 2
triSetView [ssX + 1.2, ssY + 1.3, ssZ, -1.2, -0.7, 0]
triSimFrames 20
triScreenshot "indoor_1"
triSetView [ssX, ssY + 1.3, ssZ + 1.2, 0, -0.7, -1.2]
triSimFrames 20
triScreenshot "indoor_2"
triAssert [gmHqEstablish ((gmZone (gmZoneIndex "Outpost")) select 8)]
ssA = getPosASL gmHqProp
triSetView [(ssA select 0) + 10, (ssA select 2) + 4, (ssA select 1) + 10, -8, -3, -10]
triSimFrames 20
triScreenshot "outdoor"
triEndTest
