// ============================================================================
//  Dedushka finishing: the glow of the furnace on the body itself (the engine's
//  per-unit scale of the shining faces: the traits "glow" and "farGlow").
//  Kaiju binary only. He stands with NO script set, so no light, sprite or
//  ember is in the frames: what is measured is the furnace painted on the model.
//
//  The measure is the brightest of a grid of back-buffer pixels round the
//  furnace point (triGetPixelMaxChannel, 0 to 255) and their mean.
//  Soft checks, one KJG RESULT line.      run-kj.ps1 -Name kj_glow
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4900, 4080, 0]
setDate [1985, 6, 10, 23, 30]
0 setOvercast 0.25
0 setFog 0
setViewDistance 2000
showCinemaBorder false
call compile preprocessFile "kjd_params.sqf"
if (isNil "KJD_class") then { KJD_class = "UDDedushka" }
kjgC = createCenter civilian
kjgG = createGroup civilian
kaiju = objNull
kjgAt = [4790, 4004, 0]
KJD_class createUnit [kjgAt, kjgG, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
kjgNative = (format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_])
triAssert [kjgNative]
if (true) then { kaiju setDir 180; kaiju setPos kjgAt; kaiju setCaptive true; kaiju disableAI "MOVE"; kaiju stop true }
kjgFails = []
kjgChk = { if (!(_this select 0)) then { kjgFails = kjgFails + [_this select 1]; logInfo format ["KJG CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJG ok: %1", _this select 1] } }
kjgCam = "camera" camCreate [4790, 3880, 3]
kjgCam cameraEffect ["internal", "back"]
kjgCam camSetTarget (getPos kaiju)
kjgCam camSetFov 0.7
kjgCam camCommit 0
KJD_probe = "HeliHEmpty" createVehicle [4900, 4100, 0]
kjgPt = (KJD_class GetVehicleParam "kjFurnacePoint")
// the view from a spot [x, y, eye height] at the furnace point in his pose
kjgView = { private ["_g", "_f"]; KJD_probe setPos [_this select 0, _this select 1, 0]; _g = ((getPosASL KJD_probe) select 2) + (_this select 2); _f = kjPoint [kaiju, kjgPt]; if ((count _f) != 3) then { _f = [(getPosASL kaiju) select 0, (getPosASL kaiju) select 1, ((getPosASL kaiju) select 2) + 27] }; [_this select 0, _g, _this select 1, (_f select 0) - (_this select 0), (_f select 2) - _g, (_f select 1) - (_this select 1)] }
// [half width, half height] of the grid in screen shares -> [brightest, mean]
kjgMeasure = { private ["_m", "_s", "_n", "_v", "_u", "_w"]; _m = 0; _s = 0; _n = 0; _w = _this; { _u = _x; { _v = triGetPixelMaxChannel [0.5 + (_w select 0) * _u, 0.5 + (_w select 1) * _x]; if (_v > _m) then { _m = _v }; _s = _s + _v; _n = _n + 1 } forEach [-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1] } forEach [-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1]; [_m, _s / _n] }
logInfo format ["KJG creature %1, furnace point %2 at %3, glow trait from the config %4, farGlow %5", KJD_class, kjgPt, kjPoint [kaiju, kjgPt], kjGet [kaiju, "glow"], kjGet [kaiju, "farGlow"]]

// ---- 1. by night, from 125 m in front of him -----------------------------------
triSetView ([4790, 3880, 1.7] call kjgView)
triSimFrames 20
triScreenshot "00_warmup_discard"
kjSet [kaiju, "glow", -1]
triSimFrames 4
kjgStock = [0.06, 0.1] call kjgMeasure
triScreenshot "g1_night_stock_material"
kjSet [kaiju, "glow", 1]
triSimFrames 4
kjgOne = [0.06, 0.1] call kjgMeasure
triScreenshot "g2_night_glow_1"
kjSet [kaiju, "glow", 0.35]
triSimFrames 4
kjgLow = [0.06, 0.1] call kjgMeasure
triScreenshot "g3_night_glow_035"
kjSet [kaiju, "glow", 0]
triSimFrames 4
kjgZero = [0.06, 0.1] call kjgMeasure
triScreenshot "g4_night_glow_0_dark"
kjSet [kaiju, "glow", 2]
triSimFrames 4
kjgHot = [0.06, 0.1] call kjgMeasure
triScreenshot "g5_night_glow_2_white_hot"
logInfo format ["KJG night at 125 m [brightest, mean]: stock %1, glow 1 %2, glow 0.35 %3, glow 0 %4, glow 2 %5", kjgStock, kjgOne, kjgLow, kjgZero, kjgHot]
[(kjgOne select 0) >= 200, format ["night: at glow 1 the furnace is lit on the body with no light near it (brightest %1 of 255)", kjgOne select 0]] call kjgChk
[(kjgZero select 0) <= 40, format ["night: at glow 0 the ribs are dark (brightest %1)", kjgZero select 0]] call kjgChk
[((kjgLow select 1) > (kjgZero select 1)) && ((kjgLow select 1) < (kjgOne select 1)), format ["night: glow 0.35 lies between dark and full (means %1, %2, %3)", kjgZero select 1, kjgLow select 1, kjgOne select 1]] call kjgChk
[(kjgHot select 1) > ((kjgOne select 1) * 1.08), format ["night: glow 2 is hotter than glow 1 (means %1 against %2)", kjgHot select 1, kjgOne select 1]] call kjgChk
[(kjGet [kaiju, "glow"]) == 2, "the glow trait reads back"] call kjgChk

// ---- 2. by night from 2.3 km, past the view distance (the far pass) ------------
kjgCam camSetFov 0.06
kjgCam camCommit 0
kjSet [kaiju, "glow", 1]
kjSet [kaiju, "farGlow", 0]
triSetView ([4790, 1700, 1.7] call kjgView)
triSimFrames 12
kjgFar0 = [0.03, 0.05] call kjgMeasure
kjgState0 = kjFarState kaiju
triScreenshot "g6_night_far_glow_1_farglow_0"
kjSet [kaiju, "farGlow", 0.9]
triSimFrames 6
kjgFar1 = [0.03, 0.05] call kjgMeasure
kjgState1 = kjFarState kaiju
triScreenshot "g7_night_far_glow_1_farglow_09"
kjSet [kaiju, "glow", 0]
triSimFrames 6
kjgFarDark = [0.03, 0.05] call kjgMeasure
triScreenshot "g8_night_far_glow_0"
kjSet [kaiju, "glow", -1]
triSimFrames 6
kjgFarStock = [0.03, 0.05] call kjgMeasure
triScreenshot "g9_night_far_stock_material"
logInfo format ["KJG night at 2.3 km [brightest, mean]: farGlow 0 %1 (far state %2), farGlow 0.9 %3 (far state %4), glow 0 %5, stock material %6", kjgFar0, kjgState0, kjgFar1, kjgState1, kjgFarDark, kjgFarStock]
[(kjgState1 select 0) == 2, "far: he is drawn by the far pass from 2.3 km"] call kjgChk
[(kjgFar1 select 0) >= 150, format ["far: by night the furnace shows from 2.3 km (brightest %1 of 255)", kjgFar1 select 0]] call kjgChk
[(kjgFar1 select 0) > (kjgFar0 select 0), format ["far: the night floor raises it (%1 against %2)", kjgFar1 select 0, kjgFar0 select 0]] call kjgChk
[(kjgFarDark select 0) <= 40, format ["far: at glow 0 nothing shows (brightest %1)", kjgFarDark select 0]] call kjgChk
[(kjgState1 select 2) > (kjgState0 select 2), format ["far: the fog floor by night is %1 with farGlow, %2 without", kjgState1 select 2, kjgState0 select 2]] call kjgChk

// ---- 3. by day, from 125 m: a dead furnace is dark in daylight too ------------
skipTime (36 - daytime)
kjgCam camSetFov 0.7
kjgCam camCommit 0
kjSet [kaiju, "glow", 1]
triSetView ([4790, 3880, 1.7] call kjgView)
triSimFrames 20
kjgDay1 = [0.012, 0.02] call kjgMeasure
triScreenshot "g10_day_glow_1"
kjSet [kaiju, "glow", 0]
triSimFrames 4
kjgDay0 = [0.012, 0.02] call kjgMeasure
triScreenshot "g11_day_glow_0_dark"
kjSet [kaiju, "glow", -1]
triSimFrames 4
kjgDayStock = [0.012, 0.02] call kjgMeasure
triScreenshot "g12_day_stock_material"
logInfo format ["KJG day at 125 m, the core of the furnace only [brightest, mean]: glow 1 %1, glow 0 %2, stock material %3", kjgDay1, kjgDay0, kjgDayStock]
[(kjgDay0 select 1) < ((kjgDay1 select 1) * 0.5), format ["day: at glow 0 the core of the furnace is dark in daylight too (means %1 against %2)", kjgDay0 select 1, kjgDay1 select 1]] call kjgChk
kjClear kaiju
logInfo format ["KJG RESULT: %1 checks failed: %2", count kjgFails, kjgFails]
triEndTest
