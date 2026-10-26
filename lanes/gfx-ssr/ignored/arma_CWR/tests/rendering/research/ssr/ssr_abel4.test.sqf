// SSR research captures, Abel. Three-way: classic | remastered | remastered + SSR,
// plus debug shots of the surface tags. Research copy (gfx-ssr), not committed.
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
triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present;track=ssr" call capLog }

// bay_offshore: from a boat 300 m off the east-coast shore [9240, 6950], looking W (272)
// across ~500 m of calm bay at hills rising to ~190 m. 06:30 on 5 July, sun 19.9 deg at az 76 behind.
["bay_offshore", 9240, 6950, 3, 272, -3, [1985, 7, 5, 6, 30], 0, 0, 3000] call capBegin
call {
    [["BoatW", "BoatE", "Boat"], 45, 10, 70, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__classic"
"bay_offshore__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__remastered"
"bay_offshore__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__ssr"
"bay_offshore__ssr" call capRecord
["ssrcalm", ["ssr=1", "ssrDebug=0", "ssrWave=0.35"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__ssrcalm"
"bay_offshore__ssrcalm" call capRecord
["seq", ["ssr=1", "ssrDebug=0", "ssrWave=0.35"]] call capModeX
triSimFrames 90
[] call capReadTimers
call { capSeqT0 = capSimT }
call { capSimT = capSeqT0 + 0.000 }
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__seq0"
"bay_offshore__seq0" call capRecord
call { capSimT = capSeqT0 + 0.250 }
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__seq1"
"bay_offshore__seq1" call capRecord
call { capSimT = capSeqT0 + 0.500 }
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__seq2"
"bay_offshore__seq2" call capRecord
call { capSimT = capSeqT0 + 0.750 }
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__seq3"
"bay_offshore__seq3" call capRecord
call { capSimT = capSeqT0 }
["ssrcalmd2", ["ssr=1", "ssrDebug=2", "ssrWave=0.35"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__ssrcalmd2"
"bay_offshore__ssrcalmd2" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWave=1"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__ssrtags"
"bay_offshore__ssrtags" call capRecord
["ssrhits", ["ssr=1", "ssrDebug=3"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "bay_offshore__ssrhits"
"bay_offshore__ssrhits" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssrWave=1"; capKnob = triSetLighting "ssr=0" }
[] call capClear
capRain = 0

// coastline_sunset: the committed view.
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__classic"
"coastline_sunset__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__remastered"
"coastline_sunset__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__ssr"
"coastline_sunset__ssr" call capRecord
["ssrcalm", ["ssr=1", "ssrDebug=0", "ssrWave=0.35"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__ssrcalm"
"coastline_sunset__ssrcalm" call capRecord
["ssrw06", ["ssr=1", "ssrDebug=0", "ssrWave=1", "ssrWater=0.6"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__ssrw06"
"coastline_sunset__ssrw06" call capRecord
call { capKnob = triSetLighting "ssrWater=1" }
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWave=1"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__ssrtags"
"coastline_sunset__ssrtags" call capRecord
["ssrhits", ["ssr=1", "ssrDebug=3"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__ssrhits"
"coastline_sunset__ssrhits" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssrWave=1"; capKnob = triSetLighting "ssr=0" }
[] call capClear
capRain = 0

// town_rain: Saint Louis by day in rain, on the road looking NW (320) into the town centre.
capRain = 1
["town_rain", 7162, 8931, 1.75, 320, -4, [1985, 7, 5, 14, 30], 0.85, 0, 900] call capBegin
call {
    [["Jeep"], 14, 1, 20, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_rain__classic"
"town_rain__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_rain__remastered"
"town_rain__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_rain__ssr"
"town_rain__ssr" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=1"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_rain__ssrwet"
"town_rain__ssrwet" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWet=1"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_rain__ssrtags"
"town_rain__ssrtags" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssrWave=1"; capKnob = triSetLighting "ssr=0" }
[] call capClear
capRain = 0

// town_wet_dusk: the same street after rain, broken overcast, 19:20 (dusk light, wet).
["town_wet_dusk", 7162, 8931, 1.75, 320, -4, [1985, 7, 5, 19, 20], 0.4, 0, 900] call capBegin
call {
    [["Jeep"], 14, 1, 20, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_wet_dusk__classic"
"town_wet_dusk__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_wet_dusk__remastered"
"town_wet_dusk__remastered" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=1"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "town_wet_dusk__ssrwet"
"town_wet_dusk__ssrwet" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssrWave=1"; capKnob = triSetLighting "ssr=0" }
[] call capClear
capRain = 0

// street_night: the town_rain street at night after rain, lamps on, moon behind.
["street_night", 7162, 8931, 1.75, 320, -4, [1985, 2, 27, 22, 0], 0, 0, 900] call capBegin
call {
    [["Jeep"], 14, 1, 20, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "street_night__classic"
"street_night__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "street_night__remastered"
"street_night__remastered" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=1"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "street_night__ssrwet"
"street_night__ssrwet" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWet=1"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "street_night__ssrtags"
"street_night__ssrtags" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssrWave=1"; capKnob = triSetLighting "ssr=0" }
[] call capClear
capRain = 0
// town_night: lamps and a campfire, wet after rain (ssrWet 0.8).
["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__classic"
"town_night__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__remastered"
"town_night__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__ssr"
"town_night__ssr" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=0.8"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__ssrwet"
"town_night__ssrwet" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWet=0.8"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__ssrtags"
"town_night__ssrtags" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssrWave=1"; capKnob = triSetLighting "ssr=0" }
[] call capClear
capRain = 0

// lowsun_fp: the acceptance stand-in (glossy materials; wet variant).
["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__classic"
"lowsun_fp__classic" call capRecord
["remastered", ["ssr=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__remastered"
"lowsun_fp__remastered" call capRecord
["ssr", ["ssr=1", "ssrDebug=0"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssr"
"lowsun_fp__ssr" call capRecord
["ssrwet", ["ssr=1", "ssrDebug=0", "ssrWet=0.8"]] call capModeX
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssrwet"
"lowsun_fp__ssrwet" call capRecord
["ssrtags", ["ssr=1", "ssrDebug=1", "ssrWet=0.8"]] call capModeX
triSimFrames 6
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__ssrtags"
"lowsun_fp__ssrtags" call capRecord
call { capKnob = triSetLighting "ssrWet=-1"; capKnob = triSetLighting "ssrDebug=0"; capKnob = triSetLighting "ssrWave=1"; capKnob = triSetLighting "ssr=0" }
[] call capClear
capRain = 0

call { "view=done;event=done" call capLog }
triEndTest
