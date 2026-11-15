// ============================================================================
//  Dedushka photo test (experiment, not a regression test). Puts UDDedushka on
//  the south edge of Morton (Everon), facing the sea, with a T-80, a soldier,
//  two town houses and the church at its feet, and photographs it from ground
//  level at 60 m, 300 m, 1 km and 2 km, from its feet looking up, by day and
//  by night. Then it measures where the body stops drawing, walks it, wades
//  it into the sea, and kills it to show the death clip.
//
//  Needs @uddedushka (experiments/kaiju/addon/build-uddedushka.ps1). Run it
//  through experiments/kaiju/run-tri.ps1 or make-stills.ps1. Quiet: nobody is
//  armed against anybody, nobody fires, --nosound.
//
//  Site (measured with a transect probe, see kaiju-notes\pipeline\NOTES.md):
//  the town sits on a flat shelf 16.3 m above the sea; at x = 5050 the shelf
//  runs south to z = 3800, then a bank drops to the shore at z = 3750. South
//  of that is open sea for 3.5 km, so every far camera has a clear line.
//
//  Camera rules from tools/screenshots/README.md: a cutscene camera only to
//  hide the HUD and set the lens, triSetView for framing, a throwaway first
//  frame, literal screenshot labels, a triSimFrames between change and shot.
//  Each DEDSHOT / DEDPHOTO line in game_stdout.log is a measurement.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
// Close enough that the scene is simulated, out of every frame (north of the town).
player setPos [5060, 4120, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.2
0 setFog 0
setViewDistance 5000
showCinemaBorder false
// A tri run starts in a fresh user profile with object shadows off (the Scene
// default; only the Options page turns them on). Without this verb nothing in
// the frames casts a shadow, trees and houses included.
triEnableShadows
dpC = createCenter civilian
dpG = createGroup civilian
dpE = createCenter east
dpEG = createGroup east
ded = objNull
dpAt = [5055, 3870, 0]
"UDDedushka" createUnit [dpAt, dpG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushka"]
dpAnim = 0
ded addEventHandler ["AnimChanged", { dpAnim = dpAnim + 1 }]
// Hold it still, facing the sea (south), where the far cameras are.
dpHold = { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; ded setBehaviour "CARELESS"; ded stop true; ded setDir 180; ded setPos dpAt }
call dpHold
// Scale props at its feet: an empty T-80 and a soldier in front of the toes.
// The two houses (dum01) and the church (kostel3) are the town's own.
dpTank = "T80" createVehicle [5068, 3848, 0]
dpTank setDir 250
dpMan = objNull
"SoldierEB" createUnit [[5060, 3846, 0], dpEG, "dpMan = this", 0.5, "PRIVATE"]
triSimFrames 5
if (true) then { dpMan setCaptive true; dpMan disableAI "MOVE"; dpMan setCombatMode "BLUE"; dpMan setBehaviour "CARELESS"; dpMan stop true; dpMan setDir 180 }
// Ground height probe for the camera positions. Over the sea it reads the
// water surface (2.7 m ASL here), which is what an eye "at ground level" on
// the water needs. Run 1 assumed sea level 0 and shot the far frames from
// under the surface: the engine then shows the bare sea bed.
dpProbe = "HeliHEmpty" createVehicle [5055, 4200, 0]
// HUD off; the lens is set by dpFov (camSetTarget first, or camSetFov is ignored).
dpCam = "camera" camCreate [5055, 3700, 30]
dpCam cameraEffect ["internal", "back"]
dpFov = { dpCam camSetTarget [5055, 3900, 30]; dpCam camSetFov _this; dpCam camCommit 0 }
0.7 call dpFov
// dpView: [bearing from the creature (0 = north of it), distance, eye height
// above the local ground or sea, aim height above the creature's feet].
dpView = { private ["_b", "_d", "_e", "_a", "_p", "_cx", "_cz", "_g"]; _b = _this select 0; _d = _this select 1; _e = _this select 2; _a = _this select 3; _p = getPosASL ded; _cx = (_p select 0) + _d * sin _b; _cz = (_p select 1) + _d * cos _b; dpProbe setPos [_cx, _cz, 0]; _g = (getPosASL dpProbe) select 2; dpLast = [_cx, _cz, _g, _d]; [_cx, _g + _e, _cz, (_p select 0) - _cx, ((_p select 2) + _a) - (_g + _e), (_p select 1) - _cz] }
dpNote = { logInfo format ["DEDSHOT %1 cam [x,z,ground,dist] %2 viewDistance %3 ded asl %4 move %5 dammage %6 alive %7 animChanges %8", _this, dpLast, dpVD, getPosASL ded, getMove ded, getDammage ded, alive ded, dpAnim] }
dpVD = 5000
// Sky, lighting and pose settle.
triSimFrames 200
logInfo format ["DEDPHOTO placed: pos %1 asl %2 dir %3 move %4 alive %5 tank asl %6 man asl %7", getPos ded, getPosASL ded, getDir ded, getMove ded, alive ded, getPosASL dpTank, getPosASL dpMan]
triSetView ([180, 60, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "00_warmup_discard"
// -- daylight, view distance 5000 (objects draw to 3000) ----------------------
triSetView ([180, 60, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "d01_day_60m"
"d01_day_60m" call dpNote
triSetView ([180, 150, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "d01b_day_150m"
"d01b_day_150m" call dpNote
triSetView ([180, 300, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "d02_day_300m"
"d02_day_300m" call dpNote
triSetView ([180, 450, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "d02b_day_450m"
"d02b_day_450m" call dpNote
triSetView ([180, 1000, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "d03_day_1km"
"d03_day_1km" call dpNote
triSetView ([180, 2000, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "d04_day_2km"
"d04_day_2km" call dpNote
// From its feet, looking up: 14 m in front of the origin, and between the feet.
triSetView ([180, 14, 1.7, 34] call dpView)
triSimFrames 6
triScreenshot "d05_day_feet_up"
"d05_day_feet_up" call dpNote
triSetView ([180, 1, 1.7, 30] call dpView)
triSimFrames 6
triScreenshot "d06_day_between_feet"
"d06_day_between_feet" call dpNote
// The scale group: tank, soldier, houses and church against the legs.
triSetView ([150, 45, 2.5, 6] call dpView)
triSimFrames 6
triScreenshot "d07_day_scale_props"
"d07_day_scale_props" call dpNote
// Across the town from the north (its back), the way a player in town sees it.
triSetView ([350, 230, 1.7, 22] call dpView)
triSimFrames 6
triScreenshot "d08_day_from_town_230m"
"d08_day_from_town_230m" call dpNote
// From the shelf behind its left shoulder, for the silhouette and the ground
// contact of both feet (east and west of it the ground falls away).
triSetView ([330, 100, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "d09_day_back_left_100m"
"d09_day_back_left_100m" call dpNote
// Long lens on the same far cameras: which LOD is it (head colour), and detail.
0.1 call dpFov
triSetView ([180, 300, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "z02_day_300m_zoom"
"z02_day_300m_zoom" call dpNote
triSetView ([180, 600, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "z02b_day_600m_zoom"
"z02b_day_600m_zoom" call dpNote
triSetView ([180, 1000, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "z03_day_1km_zoom"
"z03_day_1km_zoom" call dpNote
triSetView ([180, 1500, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "z03b_day_1500m_zoom"
"z03b_day_1500m_zoom" call dpNote
triSetView ([180, 2000, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "z04_day_2km_zoom"
"z04_day_2km_zoom" call dpNote
// -- where does it stop drawing? ---------------------------------------------
// View distance 5000: objects should draw to 3000 m.
triSetView ([180, 2500, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v5000_2500m_zoom"
"v5000_2500m_zoom" call dpNote
triSetView ([180, 2900, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v5000_2900m_zoom"
"v5000_2900m_zoom" call dpNote
triSetView ([180, 3100, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v5000_3100m_zoom"
"v5000_3100m_zoom" call dpNote
triSetView ([180, 3400, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v5000_3400m_zoom"
"v5000_3400m_zoom" call dpNote
// View distance 2000: objects to 1333 m.
setViewDistance 2000
dpVD = 2000
triSimFrames 30
triSetView ([180, 1250, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v2000_1250m_zoom"
"v2000_1250m_zoom" call dpNote
triSetView ([180, 1420, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v2000_1420m_zoom"
"v2000_1420m_zoom" call dpNote
// The default view distance, 900: objects to 600 m.
setViewDistance 900
dpVD = 900
triSimFrames 30
triSetView ([180, 550, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v0900_550m_zoom"
"v0900_550m_zoom" call dpNote
triSetView ([180, 650, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v0900_650m_zoom"
"v0900_650m_zoom" call dpNote
0.7 call dpFov
triSetView ([180, 550, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "v0900_550m"
"v0900_550m" call dpNote
setViewDistance 5000
dpVD = 5000
// -- shadows ----------------------------------------------------------------
// 16:30, sun in the west at about 35 degrees: the shadow should fall 55 m to
// the east across the shelf. Two frames from above (the only ones not at
// ground level): from 60 m up, 120 m out, inside the 500 m shadow range, so
// houses, trees and the tank show what a shadow looks like here.
setDate [1985, 6, 10, 16, 30]
triSimFrames 200
triSetView ([200, 120, 60, 2] call dpView)
triSimFrames 6
triScreenshot "s01_shadow_1630_from_above"
"s01_shadow_1630_from_above" call dpNote
triSetView ([180, 60, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "s02_1630_60m"
"s02_1630_60m" call dpNote
// 19:30, the sun almost down: the long-shadow case.
setDate [1985, 6, 10, 19, 30]
triSimFrames 200
triSetView ([200, 120, 60, 2] call dpView)
triSimFrames 6
triScreenshot "s03_shadow_1930_from_above"
"s03_shadow_1930_from_above" call dpNote
// -- walk -------------------------------------------------------------------
// East along the shelf, 45 m (about two and a half strides of 17.6 m).
setDate [1985, 6, 10, 12, 0]
triSimFrames 200
if (true) then { ded stop false; ded enableAI "MOVE"; ded setSpeedMode "FULL"; dpW0 = getPosASL ded; dpT0 = time; ded doMove [5100, 3870, 0] }
triSimUntil { time > dpT0 + 6 }
logInfo format ["DEDPHOTO walk t+6: moved %1 m speed %2 km/h move %3 dir %4 animChanges %5", ded distance dpW0, speed ded, getMove ded, getDir ded, dpAnim]
triSetView ([180, 75, 1.7, 18] call dpView)
triSimFrames 3
triScreenshot "w01_walk_side"
"w01_walk_side" call dpNote
triSimUntil { time > dpT0 + 12 }
logInfo format ["DEDPHOTO walk t+12: moved %1 m speed %2 km/h move %3 dir %4 animChanges %5", ded distance dpW0, speed ded, getMove ded, getDir ded, dpAnim]
triSetView ([180, 75, 1.7, 18] call dpView)
triSimFrames 3
triScreenshot "w02_walk_side"
"w02_walk_side" call dpNote
triSimUntil { time > dpT0 + 26 }
logInfo format ["DEDPHOTO walk t+26: moved %1 m speed %2 km/h move %3 dir %4 pos %5 animChanges %6", ded distance dpW0, speed ded, getMove ded, getDir ded, getPosASL ded, dpAnim]
call dpHold
triSimFrames 60
// -- night ------------------------------------------------------------------
setDate [1985, 6, 10, 23, 30]
triSimFrames 300
logInfo format ["DEDPHOTO night: daytime %1 move %2", daytime, getMove ded]
triSetView ([180, 60, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "n01_night_60m"
"n01_night_60m" call dpNote
triSetView ([180, 300, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "n02_night_300m"
"n02_night_300m" call dpNote
triSetView ([180, 1000, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "n03_night_1km"
"n03_night_1km" call dpNote
triSetView ([180, 2000, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "n04_night_2km"
"n04_night_2km" call dpNote
triSetView ([180, 14, 1.7, 34] call dpView)
triSimFrames 6
triScreenshot "n05_night_feet_up"
"n05_night_feet_up" call dpNote
triSetView ([350, 230, 1.7, 22] call dpView)
triSimFrames 6
triScreenshot "n08_night_from_town_230m"
"n08_night_from_town_230m" call dpNote
0.1 call dpFov
triSetView ([180, 1000, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "nz03_night_1km_zoom"
"nz03_night_1km_zoom" call dpNote
triSetView ([180, 2000, 1.7, 20] call dpView)
triSimFrames 6
triScreenshot "nz04_night_2km_zoom"
"nz04_night_2km_zoom" call dpNote
0.7 call dpFov
triAssert [alive ded]
// -- water ------------------------------------------------------------------
// The sea surface is 2.7 m ASL. On the x = 5000 line the bed is at 2.98 m at
// z = 3810, 0.05 m at 3800 and falls 1 in 6 from there: about 1 m of water at
// 3806, 2.7 m at 3800, 7 m at 3770. A Man is hurt in more than 1.2 m
// (SoldierOldSim.cpp:1124-1136). A unit put on the sea sinks to the bed at a
// few metres per second, so each step waits. Camera: the headland 50 m east.
setDate [1985, 6, 10, 12, 0]
triSimFrames 200
ded setPos [5000, 3806, 0]
ded setDir 180
if (true) then { dpT0 = time; dpD0 = getDammage ded }
triSimUntil { time > dpT0 + 8 }
logInfo format ["DEDPHOTO water z3806: feet asl %1 (sea surface 2.7) dammage +%2 in 8 s alive %3 move %4", (getPosASL ded) select 2, (getDammage ded) - dpD0, alive ded, getMove ded]
triSetView ([108, 52, 1.7, 12] call dpView)
triSimFrames 6
triScreenshot "x01_water_1m"
"x01_water_1m" call dpNote
ded setPos [5000, 3800, 0]
if (true) then { dpT0 = time; dpD0 = getDammage ded }
triSimUntil { time > dpT0 + 8 }
logInfo format ["DEDPHOTO water z3800: feet asl %1 (sea surface 2.7) dammage +%2 in 8 s alive %3 move %4", (getPosASL ded) select 2, (getDammage ded) - dpD0, alive ded, getMove ded]
triSetView ([100, 54, 1.7, 12] call dpView)
triSimFrames 6
triScreenshot "x02_water_3m"
"x02_water_3m" call dpNote
ded setPos [5000, 3770, 0]
if (true) then { dpT0 = time; dpD0 = getDammage ded }
triSimUntil { time > dpT0 + 10 }
logInfo format ["DEDPHOTO water z3770: feet asl %1 (sea surface 2.7) dammage +%2 in 10 s alive %3 move %4", (getPosASL ded) select 2, (getDammage ded) - dpD0, alive ded, getMove ded]
triSetView ([75, 60, 1.7, 12] call dpView)
triSimFrames 6
triScreenshot "x03_water_7m"
"x03_water_7m" call dpNote
// Walking in it: 20 m west along the shore line, in about 3 m of water.
if (true) then { ded stop false; ded enableAI "MOVE"; dpW0 = getPosASL ded; dpT0 = time; dpD0 = getDammage ded; ded doMove [4980, 3800, 0] }
triSimUntil { time > dpT0 + 12 }
logInfo format ["DEDPHOTO water walk: from %1 to %2 in 12 s, move %3 dammage +%4 alive %5", dpW0, getPosASL ded, getMove ded, (getDammage ded) - dpD0, alive ded]
triAssert [alive ded]
// -- death ------------------------------------------------------------------
// Back on the shelf: the topple clip (5 s) and the dead pose.
call dpHold
triSimFrames 60
if (true) then { dpT0 = time; ded setDammage 1 }
triSimUntil { time > dpT0 + 2.5 }
triSetView ([135, 95, 1.7, 14] call dpView)
triSimFrames 3
triScreenshot "y01_dying"
"y01_dying" call dpNote
triSimUntil { time > dpT0 + 9 }
triSetView ([135, 95, 1.7, 6] call dpView)
triSimFrames 6
triScreenshot "y02_dead"
"y02_dead" call dpNote
logInfo format ["DEDPHOTO end: time %1 alive %2 move %3 dammage %4 animChanges %5", time, alive ded, getMove ded, getDammage ded, dpAnim]
triEndTest
