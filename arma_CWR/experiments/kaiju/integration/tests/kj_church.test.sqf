// ============================================================================
//  Dedushka finishing: a destroyed church shows its collapse (engine request 4:
//  Church::Animate never called the base, where a destroyed object is morphed
//  to its collapsed shape). Kaiju binary. With a kaiju unit in the world the
//  church of Morton is destroyed with credited damage and photographed before
//  and after; the picture of the church must have changed (its collapsed shape:
//  the nave sunk and broken, the tower leaning).
//      run-kj.ps1 -Name kj_church
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4900, 4080, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.2
0 setFog 0
setViewDistance 2000
showCinemaBorder false
call compile preprocessFile "kjd_params.sqf"
if (isNil "KJD_class") then { KJD_class = "UDDedushka" }
kjcC = createCenter civilian
kjcG = createGroup civilian
kaiju = objNull
KJD_class createUnit [[4960, 3990, 0], kjcG, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
kjcNative = (format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_])
triAssert [kjcNative]
if (true) then { kaiju setDir 90; kaiju setCaptive true; kaiju disableAI "MOVE"; kaiju stop true }
kjcFails = []
kjcChk = { if (!(_this select 0)) then { kjcFails = kjcFails + [_this select 1]; logInfo format ["KJC CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJC ok: %1", _this select 1] } }
// the church: the heaviest map object near where the crush feature found it
kjcChurch = objNull
kjcMass = 0
{ if ((("AllVehicles" countType [_x]) == 0) && ((getMass _x) > kjcMass)) then { kjcMass = getMass _x; kjcChurch = _x } } forEach (nearestObjects [[5057, 3925, 0], [], 45])
logInfo format ["KJC the church: %1, type %2, mass %3, at %4, damage %5, kjVersion %6", kjcChurch, typeOf kjcChurch, kjcMass, getPos kjcChurch, getDammage kjcChurch, kjVersion]
[!(isNull kjcChurch), "a church was found in Morton"] call kjcChk
kjcCam = "camera" camCreate [5000, 3900, 3]
kjcCam cameraEffect ["internal", "back"]
kjcCam camSetTarget (getPos kjcChurch)
kjcCam camSetFov 0.7
kjcCam camCommit 0
kjcP = getPosASL kjcChurch
// from 70 m to the south-west, aimed 14 m up the tower
triSetView [(kjcP select 0) - 50, (kjcP select 2) + 3, (kjcP select 1) - 50, 50, 11, 50]
triSimFrames 20
triScreenshot "00_warmup_discard"
// the picture of the church: a grid of pixels over the middle of the frame, where it stands
kjcCol = { private ["_a", "_u"]; _a = []; { _u = _x; { _a = _a + [triGetPixelMaxChannel [_u, _x]] } forEach [0.46, 0.5, 0.54, 0.58, 0.62, 0.66, 0.7, 0.74, 0.78, 0.82] } forEach [0.4, 0.43, 0.46, 0.49, 0.52, 0.55, 0.58, 0.61, 0.64]; _a }
kjcBefore = call kjcCol
triScreenshot "c1_the_church_stands"
kjDamage [kjcChurch, kaiju, 0, 200000, 8]
if ((getDammage kjcChurch) < 1) then { kjcChurch setDammage 1 }
triSimFrames 30
triScreenshot "c2_a_moment_after"
if (true) then { kjcT = time }
triSimUntil { time > kjcT + 12 }
kjcAfter = call kjcCol
triScreenshot "c3_the_church_is_down"
kjcDiff = 0
kjcI = 0
{ if ((abs (_x - (kjcBefore select kjcI))) > 25) then { kjcDiff = kjcDiff + 1 }; kjcI = kjcI + 1 } forEach kjcAfter
logInfo format ["KJC the picture of the church: %1 of %2 samples changed by more than 25 of 255; damage %3", kjcDiff, count kjcBefore, getDammage kjcChurch]
[(getDammage kjcChurch) >= 1, "the church is destroyed"] call kjcChk
[kjcDiff >= 8, format ["the church has come down in the picture (%1 of %2 samples over it changed)", kjcDiff, count kjcBefore]] call kjcChk
kjClear kaiju
logInfo format ["KJC RESULT: %1 checks failed: %2", count kjcFails, kjcFails]
triEndTest
