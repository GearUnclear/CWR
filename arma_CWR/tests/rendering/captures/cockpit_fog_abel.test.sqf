// Fog in vehicle cockpits (Remastered lighting): the world seen out of a
// cockpit is fogged like the outside view, since AI sight is cut by that air.
// A heli pilot view (through the canopy glass) and a jeep driver view
// (through the windscreen) are each counted on a clear day and again in
// weather fog: the dark samples of the far tree line in a band at the horizon
// must mostly fade into the fog, and the dark instrument panel must stay dark
// (not fogged; its exact level follows the exposure, which the fog moves).
// Fails when the interior views draw no fog (the glass hid the world from the
// fog, or no fog ran in the interior). A count over a band, not one pixel, so
// the vehicle settling by a few pixels does not move the result. Positions are
// for 960x640 and the views below; the bounds are set from runs with and
// without the fog in the cockpit (heli 2 vs 17-38 dark samples in fog, jeep
// 21-22 vs 61-66).
#include "capture_lib.sqf"
capWorld = "Abel"
capView = "cockpit_fog"
capOX = 7300
capOZ = 10900
// The samples under cfThr (max channel) in the band cfUs x cfVs.
cfCount = { cfThr = _this; cfDark = 0; { cfV = _x; { if ((triGetPixelMaxChannel [_x, cfV]) < cfThr) then { cfDark = cfDark + 1 } } forEach cfUs } forEach cfVs; cfDark }
cfLog = { logInfo (format ["COCKPITFOG;%1", _this]) }
triSimUntil { alive player }
triCaptureAtPresent true
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false
capProbe = "HeliHEmpty" createVehicle [capOX, capOZ, 0]
capCam = "camera" camCreate [capOX, capOZ, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 60
capFov = 0.7
capSimT = 0
capPinTime = false
capProps = []
["cockpit_fog", capOX, capOZ, 2.2, 152.5, 0, [1985, 6, 10, 11, 0], 0.2, 0, 1500] call capBegin
triSimFrames 30
triAssertEq [(triSetLighting "remastered"), "OK"]
call { capUseView = false; capCam cameraEffect ["terminate", "back"]; capClearRes = triClearView; cfHeli = "UH60" createVehicle [capOX, capOZ, 0]; cfHeli setDir 152.5; cfHeli setPos [capOX, capOZ, 0]; cfHeli allowDamage false; player moveInDriver cfHeli }
triSimFrames 60

// The heli pilot: the far tree line through the canopy, and the panel.
call { (vehicle player) switchCamera "INTERNAL"; capFog = 0; [] call capPinEnv }
triSimFrames 120
triScreenshot "cockpit_fog_1_heli_clear"
call { cfUs = [0.5, 0.508, 0.516, 0.524, 0.532, 0.54, 0.548, 0.556, 0.564, 0.572, 0.58, 0.588, 0.596, 0.604, 0.612, 0.62, 0.628, 0.636, 0.644, 0.652, 0.66, 0.668, 0.676, 0.684, 0.692, 0.7, 0.708, 0.716, 0.724, 0.732, 0.74, 0.748, 0.756, 0.764]; cfVs = [0.48, 0.488, 0.496, 0.504, 0.512, 0.52, 0.528, 0.536]; cfHeliClear = 100 call cfCount; format ["heliClear=%1", cfHeliClear] call cfLog }
call { capFog = 0.6; [] call capPinEnv }
triSimFrames 120
triScreenshot "cockpit_fog_2_heli_fog"
call { cfHeliFog = 100 call cfCount; format ["heliFog=%1", cfHeliFog] call cfLog }
triAssertGe [cfHeliClear, 20]
triAssertLe [cfHeliFog, 8]
triAssertLe [(triGetPixelMaxChannel [0.40, 0.88]), 70]

// The jeep driver: the far tree line through the windscreen.
call { player action ["GETOUT", vehicle player] }
triSimFrames 60
call { deleteVehicle cfHeli; cfJeep = "Jeep" createVehicle [capOX, capOZ, 0]; cfJeep setDir 152.5; cfJeep setPos [capOX, capOZ, 0]; cfJeep allowDamage false; player moveInDriver cfJeep }
triSimFrames 60
call { (vehicle player) switchCamera "INTERNAL"; capFog = 0; [] call capPinEnv }
triSimFrames 120
triScreenshot "cockpit_fog_3_jeep_clear"
call { cfUs = [0.32, 0.332, 0.344, 0.356, 0.368, 0.38, 0.392, 0.404, 0.416, 0.428, 0.44, 0.452, 0.464, 0.476, 0.488, 0.5, 0.512, 0.524, 0.536, 0.548, 0.56, 0.572, 0.584, 0.596, 0.608, 0.62, 0.632, 0.644, 0.656, 0.668, 0.68, 0.692, 0.704, 0.716, 0.728, 0.74, 0.752, 0.764, 0.776, 0.788, 0.8, 0.812, 0.824, 0.836, 0.848, 0.86, 0.872, 0.884, 0.896, 0.908, 0.92]; cfVs = [0.17, 0.178, 0.186, 0.194, 0.202, 0.21, 0.218, 0.226, 0.234, 0.242, 0.25]; cfJeepClear = 80 call cfCount; format ["jeepClear=%1", cfJeepClear] call cfLog }
call { capFog = 0.6; [] call capPinEnv }
triSimFrames 120
triScreenshot "cockpit_fog_4_jeep_fog"
call { cfJeepFog = 80 call cfCount; format ["jeepFog=%1", cfJeepFog] call cfLog }
triAssertGe [cfJeepClear, 100]
triAssertLe [cfJeepFog, 40]
triEndTest
