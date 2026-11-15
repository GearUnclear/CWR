// ============================================================================
//  Dedushka finishing: the far draw out to its limit (6 km), by day and by
//  night. He stands dormant west of Morton with NO script set (the glow of the
//  furnace and the night floor are set by hand, so nothing but the engine is
//  in the frames). The camera goes out over the bay to the south, 30 m above
//  the sea, through a long lens: 2.3, 3, 4, 5, 5.5 and 6 km. (South of 4 km it
//  is past the edge of the map, over open sea.)
//
//  Checks, soft, one KJR RESULT line: he is drawn by the far pass at every
//  distance inside the limit; the fog floor falls with distance and is 0 at
//  the limit (the far draw fades out there by design: nothing pops); by night
//  the furnace still reads at 5 km.              run-kj.ps1 -Name kj_far
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4900, 4080, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.25
0 setFog 0
setViewDistance 2000
showCinemaBorder false
call compile preprocessFile "kjd_params.sqf"
if (isNil "KJD_class") then { KJD_class = "UDDedushka" }
kjrC = createCenter civilian
kjrG = createGroup civilian
kaiju = objNull
kjrAt = [4790, 4004, 0]
KJD_class createUnit [kjrAt, kjrG, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
kjrNative = (format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_])
triAssert [kjrNative]
if (true) then { kaiju setDir 180; kaiju setPos kjrAt; kaiju setCaptive true; kaiju disableAI "MOVE"; kaiju stop true; kjSet [kaiju, "glow", 0.8]; kjSet [kaiju, "farGlow", 0.9] }
kjrFails = []
kjrChk = { if (!(_this select 0)) then { kjrFails = kjrFails + [_this select 1]; logInfo format ["KJR CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJR ok: %1", _this select 1] } }
kjrCam = "camera" camCreate [4790, 3880, 3]
kjrCam cameraEffect ["internal", "back"]
kjrCam camSetTarget (getPos kaiju)
kjrCam camSetFov 0.06
kjrCam camCommit 0
kjrPt = (KJD_class GetVehicleParam "kjFurnacePoint")
kjrLimit = kjGet [kaiju, "farDraw"]
// the view from so many metres south of him, 30 m above the sea, at his furnace point
kjrView = { private ["_f"]; _f = kjPoint [kaiju, kjrPt]; if ((count _f) != 3) then { _f = [(getPosASL kaiju) select 0, (getPosASL kaiju) select 1, ((getPosASL kaiju) select 2) + 27] }; [_f select 0, 30, (_f select 1) - _this, 0, (_f select 2) - 30, _this] }
// the brightest and the mean of a grid of pixels round the middle of the frame
kjrMeasure = { private ["_m", "_s", "_n", "_v", "_u"]; _m = 0; _s = 0; _n = 0; { _u = _x; { _v = triGetPixelMaxChannel [0.5 + 0.02 * _u, 0.5 + 0.035 * _x]; if (_v > _m) then { _m = _v }; _s = _s + _v; _n = _n + 1 } forEach [-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1] } forEach [-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1]; [_m, _s / _n] }
logInfo format ["KJR creature %1, far limit %2 m, view distance %3", KJD_class, kjrLimit, triGetViewDistance]
triSetView (2300 call kjrView)
triSimFrames 20
triScreenshot "00_warmup_discard"
kjrLog = []
// [metres, name of the frame] call kjrShot
kjrShot = { triSetView ((_this select 0) call kjrView) }
kjrNote = { kjrLog = kjrLog + [[_this select 0, kjFarState kaiju, call kjrMeasure]]; logInfo format ["KJR %1 at %2 m: far state %3 (mode, metres, fog floor, frames normal, frames far), middle of the frame [brightest, mean] %4", _this select 1, _this select 0, kjFarState kaiju, call kjrMeasure] }

// ---- by day -------------------------------------------------------------------------
[2300, "day"] call kjrShot
triSimFrames 8
[2300, "day"] call kjrNote
triScreenshot "d1_day_2300m"
kjrCam camSetFov 0.7
kjrCam camCommit 0
triSimFrames 8
triScreenshot "d1b_day_2300m_the_naked_eye"
kjrCam camSetFov 0.06
kjrCam camCommit 0
[3000, "day"] call kjrShot
triSimFrames 8
[3000, "day"] call kjrNote
triScreenshot "d2_day_3000m"
[4000, "day"] call kjrShot
triSimFrames 8
[4000, "day"] call kjrNote
triScreenshot "d3_day_4000m"
[5000, "day"] call kjrShot
triSimFrames 8
[5000, "day"] call kjrNote
triScreenshot "d4_day_5000m"
[5500, "day"] call kjrShot
triSimFrames 8
[5500, "day"] call kjrNote
triScreenshot "d5_day_5500m"
[5950, "day"] call kjrShot
triSimFrames 8
[5950, "day"] call kjrNote
triScreenshot "d6_day_5950m_at_the_limit"
kjrDay = kjrLog
kjrLog = []

// ---- by night -----------------------------------------------------------------------
skipTime (23.5 - daytime)
triSimFrames 30
[2300, "night"] call kjrShot
triSimFrames 8
[2300, "night"] call kjrNote
triScreenshot "n1_night_2300m"
kjrCam camSetFov 0.7
kjrCam camCommit 0
triSimFrames 8
triScreenshot "n1b_night_2300m_the_naked_eye"
kjrCam camSetFov 0.06
kjrCam camCommit 0
[4000, "night"] call kjrShot
triSimFrames 8
[4000, "night"] call kjrNote
triScreenshot "n2_night_4000m"
[5000, "night"] call kjrShot
triSimFrames 8
[5000, "night"] call kjrNote
triScreenshot "n3_night_5000m"
[5500, "night"] call kjrShot
triSimFrames 8
[5500, "night"] call kjrNote
triScreenshot "n4_night_5500m"
[5950, "night"] call kjrShot
triSimFrames 8
[5950, "night"] call kjrNote
triScreenshot "n5_night_5950m_at_the_limit"
kjrNight = kjrLog

// ---- the checks ----------------------------------------------------------------------
kjrOk = true
{ if (((_x select 1) select 0) != 2) then { kjrOk = false } } forEach (kjrDay + kjrNight)
[kjrOk, "he is drawn by the far pass at every distance from 2.3 km to the limit, by day and by night"] call kjrChk
kjrOk = true
kjrPrev = 2
{ if (((_x select 1) select 2) > (kjrPrev + 0.001)) then { kjrOk = false }; kjrPrev = (_x select 1) select 2 } forEach kjrDay
[kjrOk, "by day the fog floor only falls with distance"] call kjrChk
[(((kjrDay select 2) select 1) select 2) > 0.3, format ["by day at 4 km he still has a third of his contrast or more (fog floor %1)", ((kjrDay select 2) select 1) select 2]] call kjrChk
[(((kjrDay select 5) select 1) select 2) < 0.02, format ["at the limit he has faded out (fog floor %1 at 5950 m of %2): nothing pops", ((kjrDay select 5) select 1) select 2, kjrLimit]] call kjrChk
[(((kjrNight select 2) select 2) select 0) >= 120, format ["by night the furnace still reads at 5 km (brightest %1 of 255)", ((kjrNight select 2) select 2) select 0]] call kjrChk
[(((kjrNight select 0) select 2) select 0) >= 150, format ["by night the furnace reads at 2.3 km (brightest %1 of 255)", ((kjrNight select 0) select 2) select 0]] call kjrChk
kjClear kaiju
logInfo format ["KJR RESULT: %1 checks failed: %2", count kjrFails, kjrFails]
triEndTest
