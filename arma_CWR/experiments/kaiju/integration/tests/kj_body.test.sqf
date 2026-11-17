// ============================================================================
//  Dedushka finishing: the body. He is killed through the combat feature
//  (the whole death sequence), on land, and the body is measured and
//  photographed: does it lie on the ground, are the ribs dark.
//  Kaiju binary. Soft checks, one KJY RESULT line.   run-kj.ps1 -Name kj_body
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
if (!(isNil "KJD_hour")) then { skipTime (KJD_hour - daytime) }
kjyC = createCenter civilian
kjyG = createGroup civilian
kaiju = objNull
kjyAt = [4790, 4004, 0]
KJD_class createUnit [kjyAt, kjyG, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
kjyNative = (format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_])
triAssert [kjyNative]
kjyFails = []
kjyChk = { if (!(_this select 0)) then { kjyFails = kjyFails + [_this select 1]; logInfo format ["KJY CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJY ok: %1", _this select 1] } }
if (true) then { kaiju setDir 180; kaiju setPos kjyAt; kaiju setCaptive true; KJ_brainAuto = false; KJ_ready = false; [kaiju, "idle"] exec "kj\start.sqs" }
triSimUntil { KJ_ready }
triSimFrames 60
kjyCam = "camera" camCreate [4790, 3880, 3]
kjyCam cameraEffect ["internal", "back"]
kjyCam camSetTarget (getPos kaiju)
kjyCam camSetFov 0.7
kjyCam camCommit 0
KJD_probe = "HeliHEmpty" createVehicle [4900, 4100, 0]
// the view from [x, y, eye height] at a point [x, y, height above the ground there]
kjyView = { private ["_g", "_t"]; KJD_probe setPos [_this select 0, _this select 1, 0]; _g = ((getPosASL KJD_probe) select 2) + (_this select 2); KJD_probe setPos [(_this select 3) select 0, (_this select 3) select 1, 0]; _t = ((getPosASL KJD_probe) select 2) + ((_this select 3) select 2); [_this select 0, _g, _this select 1, ((_this select 3) select 0) - (_this select 0), _t - _g, ((_this select 3) select 1) - (_this select 1)] }
kjyGround = { KJD_probe setPos [_this select 0, _this select 1, 0]; (getPosASL KJD_probe) select 2 }
kjyF0 = kjPoint [kaiju, KJ_furnacePoint]
logInfo format ["KJY standing: pos %1, furnace point %2 (%3 m above the ground under it), glow %4, engine version %5", getPosASL kaiju, kjyF0, (kjyF0 select 2) - ([kjyF0 select 0, kjyF0 select 1] call kjyGround), KJ_glowNow, KJ_engineVersion]
triSetView ([4850, 3900, 1.7, [4790, 3985, 16]] call kjyView)
triSimFrames 10
triScreenshot "00_warmup_discard"
triScreenshot "y1_alive"
if (true) then { kjyT = time; [kaiju, 100000, player] call KJCB_fnWound }
triSimUntil { KJ_dying || (time > kjyT + 5) }
[KJ_dying, "the death sequence has started"] call kjyChk
triSimUntil { (KJ_deathPhase == "flare") || (time > kjyT + 20) }
triSimUntil { (KJ_deathPhase == "vent") || (time > kjyT + 30) }
triScreenshot "y2_flare_at_its_top"
logInfo format ["KJY flare top: glow %1", kjGet [kaiju, "glow"]]
[(kjGet [kaiju, "glow"]) > 1.5, "flare: the furnace on the body has climbed to white hot"] call kjyChk
triSimUntil { (KJ_deathPhase == "dark") || (time > kjyT + 35) }
triSimFrames 20
triScreenshot "y3_dark_standing"
[(kjGet [kaiju, "glow"]) == 0, "dark: the furnace on the body is out"] call kjyChk
triSimUntil { (KJ_deathPhase == "body") || (time > kjyT + 60) }
[KJ_deathPhase == "body", "he lies still"] call kjyChk
// the dust of the fall hangs for a while: wait for it to thin
if (true) then { kjyT2 = time }
triSimUntil { time > kjyT2 + 28 }
kjyF1 = kjPoint [kaiju, KJ_furnacePoint]
kjyH1 = (kjyF1 select 2) - ([kjyF1 select 0, kjyF1 select 1] call kjyGround)
kjyHead = kjPoint [kaiju, "hlava"]
logInfo format ["KJY body: pos %1 (asl %2), alive %3, move %4, furnace point %5, %6 m above the ground under it, head %7, glow %8, lights %9, set into the ground %10 m", getPos kaiju, getPosASL kaiju, alive kaiju, getMove kaiju, kjyF1, kjyH1, kjyHead, kjGet [kaiju, "glow"], kjLightCount kaiju, kjGet [kaiju, "bodySink"]]
[kjyH1 < (0.2 * KJ_heightM), format ["the body lies on the ground: its furnace point is %1 m above it (a fifth of his height is %2 m)", kjyH1, 0.2 * KJ_heightM]] call kjyChk
[(kjGet [kaiju, "glow"]) == 0, "the ribs of the body are dark"] call kjyChk
triSetView ([4850, 3900, 1.7, [kjyF1 select 0, kjyF1 select 1, 5]] call kjyView)
triSimFrames 10
triScreenshot "y4_the_body_from_the_side"
triSetView ([(kjyF1 select 0) + 30, (kjyF1 select 1) - 75, 25, [kjyF1 select 0, kjyF1 select 1, 4]] call kjyView)
triSimFrames 10
triScreenshot "y5_the_body_from_above"
skipTime (23.5 - daytime)
triSimFrames 30
triScreenshot "y7_the_body_by_night"
// ---- the body by daylight: the next morning, from both sides and from above (stills for the sheet) ----
kjMx = ((getPos kaiju) select 0) + 0.5 * KJ_heightM * (sin (getDir kaiju))
kjMy = ((getPos kaiju) select 1) + 0.5 * KJ_heightM * (cos (getDir kaiju))
kjMz = ((getPosASL kaiju) select 2) + 12
if (KJ_native) then { if (KJ_engineVersion >= 2) then { kjMz = ((kjGround [kjMx, kjMy]) select 0) + 5; if (kjMz < 3) then { kjMz = 3 } } }
// [degrees clockwise from the way he fell, metres from the middle of the body, eye height above the ground or the sea there] call kjBodyView
kjBodyView = { private ["_b", "_cx", "_cy", "_e"]; _b = (getDir kaiju) + (_this select 0); _cx = kjMx + (_this select 1) * (sin _b); _cy = kjMy + (_this select 1) * (cos _b); KJD_probe setPos [_cx, _cy, 0]; _e = ((getPosASL KJD_probe) select 2) + (_this select 2); [_cx, _e, _cy, kjMx - _cx, kjMz - _e, kjMy - _cy] }
skipTime (31 - daytime)
triSimFrames 40
kjyCam camSetFov 0.8
kjyCam camCommit 0
triSetView ([90, 85, 20] call kjBodyView)
triSimFrames 6
triScreenshot "15_the_next_morning_the_body_from_its_right"
triSetView ([270, 85, 20] call kjBodyView)
triSimFrames 6
triScreenshot "15b_the_next_morning_the_body_from_its_left"
triSetView ([200, 60, 75] call kjBodyView)
triSimFrames 6
triScreenshot "15c_the_next_morning_the_body_from_above"
logInfo format ["KJY the next morning: the middle of the body [%1, %2], aim height %3 asl, the hour %4", kjMx, kjMy, kjMz, daytime]
logInfo format ["KJY RESULT: %1 checks failed: %2", count kjyFails, kjyFails]
if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false }
triSimFrames 10
triEndTest
