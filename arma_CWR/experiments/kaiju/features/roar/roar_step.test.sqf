// ============================================================================
//  Dedushka ROAR: the footfall and presence without sight (experiment, not a
//  regression test). Same site as roar_event (south edge of Morton, Everon).
//
//  1. One footfall with the player 150 m off: the dust ring is made, the
//     player's view moves (the shake stand-in), the shake starts before the
//     thump is heard, and the player ends where he stood.
//  2. One footfall with the player 1500 m off: no shake, the far thump class,
//     heard more than four seconds later.
//  3. He walks: footfalls come by themselves, one per half stride.
//  4. Presence at night: street lamps near him go out and stay out, lamps
//     further off are left alone, the furnace drone plays when the player is
//     near, the radio breaks into static, and the birds go up and fly away
//     from him.
//  --nosound, nobody fires. Each KJSTEPTEST line in game_stdout.log is a
//  measurement.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
setViewDistance 1500
0 setFog 0
triSimFrames 30
triAssert [!(isNull ded)]
if (true) then { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; ded setBehaviour "CARELESS"; ded stop true; ded setDir 0 }
// The roar scripts load this themselves; loading it here first means every KJ_ global the test reads exists from the start.
if (true) then { call compile preprocessFile "kj\roar\init.sqf"; [ded] call KJR_fnSize }
// The others would only wander into the frame.
if (true) then { { _x setPos [5300, 4600, 0]; _x disableAI "MOVE"; _x stop true } forEach [kjrA1, kjrA2, kjrA3, kjrA4, kjrB1, kjrB2, kjrB3, kjrC1, kjrC2, kjrC3, kjrD1]; kjrCar setPos [5320, 4620, 0] }
// -- 1. a footfall, the player 150 m from him ------------------------------------
player setPos [5058, 4020, 0]
player setDir 180
triSimFrames 40
if (true) then { stP0 = getPosASL player; stD0 = getDir player; stDev = 0; stYaw = 0 }
// At rest the player does not move by himself.
triSimFrames 20
logInfo format ["KJSTEPTEST at rest: moved %1 m, turned %2 deg, %3 m from him", (getPosASL player) distance stP0, abs ((getDir player) - stD0), player distance ded]
triAssertLt [(getPosASL player) distance stP0, 0.02]
if (true) then { stP0 = getPosASL player; stD0 = getDir player; stT0 = time; [ded] exec "kj\roar\step.sqs" }
// Every frame for 1.6 s: how far the player has been pushed from where he stood.
triSimUntil { if (((getPosASL player) distance stP0) > stDev) then { stDev = (getPosASL player) distance stP0 }; if ((abs ((getDir player) - stD0)) > stYaw) then { stYaw = abs ((getDir player) - stD0) }; time > stT0 + 1.6 }
logInfo format ["KJSTEPTEST near step: log %1 | shake %2 | sound %3", KJ_stepLog, KJ_shakeLog, KJ_sndLog]
logInfo format ["KJSTEPTEST near step: view pushed up to %1 m and %2 deg, ends %3 m and %4 deg from where it stood", stDev, stYaw, (getPosASL player) distance stP0, abs ((getDir player) - stD0)]
triAssertEq [KJ_stepCount, 1]
triAssertEq [count KJ_shakeLog, 1]
// the dust ring was made
triAssertGt [(KJ_stepLog select 0) select 5, 20]
// the view moved, and by a sane amount
triAssertGt [stDev, 0.008]
triAssertLt [stDev, 0.3]
triAssertGt [stYaw, 0.03]
// and the player is back where he stood
triAssertLt [(getPosASL player) distance stP0, 0.03]
triAssertLt [abs ((getDir player) - stD0), 0.1]
// the near thump, late by distance; the shake came first
triAssertEq [(KJ_sndLog select 0) select 0, "KJ_Step"]
triAssertNear [((KJ_sndLog select 0) select 2) - ((KJ_sndLog select 0) select 1), ((KJ_stepLog select 0) select 3) / 343, 0.05]
triAssertLt [(KJ_shakeLog select 0) select 0, (KJ_sndLog select 0) select 2]
logInfo format ["KJSTEPTEST order of arrival: shake at +%1 s, thump at +%2 s", ((KJ_shakeLog select 0) select 0) - stT0, ((KJ_sndLog select 0) select 2) - stT0]
// -- the still: the dust ring at his foot ------------------------------------------
stCam = "camera" camCreate [5050, 3960, 3]
stCam cameraEffect ["internal", "back"]
if (true) then { stCam camSetTarget [5055, 3870, 10]; stCam camSetFov 0.7; stCam camCommit 0 }
showCinemaBorder false
// From the open shelf south-east of him, 50 m from his right foot.
triSetView [5080, 18.0, 3827, -0.42, 0.12, 1]
triSimFrames 30
triScreenshot "00_warmup_discard"
if (true) then { stT0 = time; [ded] exec "kj\roar\step.sqs" }
triSimUntil { time > stT0 + 0.7 }
triScreenshot "01_footfall_dust_ring_plus_0.7s"
triSimUntil { time > stT0 + 2.5 }
triScreenshot "02_footfall_dust_ring_plus_2.5s"
triSimUntil { time > stT0 + 6 }
// -- 2. a footfall, the player 1500 m from him ---------------------------------------
if (true) then { player setPos [5060, 5370, 0]; stN = count KJ_shakeLog; stS = count KJ_sndLog }
triSimFrames 40
if (true) then { stT0 = time; stFar = player distance ded; [ded] exec "kj\roar\step.sqs" }
triSimUntil { (count KJ_sndLog) > stS }
logInfo format ["KJSTEPTEST far step: player %1 m off, sound %2, shakes %3 (was %4), step log %5", stFar, KJ_sndLog select stS, count KJ_shakeLog, stN, KJ_stepLog select ((count KJ_stepLog) - 1)]
triAssertEq [(KJ_sndLog select stS) select 0, "KJ_StepFar"]
triAssertGt [((KJ_sndLog select stS) select 2) - ((KJ_sndLog select stS) select 1), 4]
triSimFrames 30
triAssertEq [count KJ_shakeLog, stN]
// The roar from the same place: the far class too, and four seconds late.
if (true) then { stS = count KJ_sndLog; [ded, "KJ_Roar", "KJ_RoarFar", 0, KJR_MOUTH_H] exec "kj\roar\snd.sqs" }
triSimUntil { (count KJ_sndLog) > stS }
logInfo format ["KJSTEPTEST far roar: %1", KJ_sndLog select stS]
triAssertEq [(KJ_sndLog select stS) select 0, "KJ_RoarFar"]
triAssertNear [((KJ_sndLog select stS) select 2) - ((KJ_sndLog select stS) select 1), stFar / 343, 0.2]
// -- 3. he walks: footfalls by themselves -----------------------------------------------
if (true) then { player setPos [5058, 4020, 0]; player setDir 180 }
triSimFrames 20
if (true) then { stN = KJ_stepCount; stW0 = getPos ded; [ded] exec "kj\roar\walk.sqs"; ded stop false; ded enableAI "MOVE"; ded setSpeedMode "FULL"; ded doMove [5105, 3870, 0]; stT0 = time }
triSimUntil { ((ded distance stW0) > 30) or (time > stT0 + 40) }
logInfo format ["KJSTEPTEST walk: %1 m in %2 s, %3 footfalls (one per %4 m), last %5", ded distance stW0, time - stT0, KJ_stepCount - stN, 0.22 * KJR_H, KJ_stepLog select ((count KJ_stepLog) - 1)]
triAssertGt [ded distance stW0, 30]
triAssertGe [KJ_stepCount - stN, 3]
triAssertLe [KJ_stepCount - stN, 5]
triSetView [5075, 18.0, 3815, 0.1, 0.25, 1]
triSimFrames 3
triScreenshot "03_walking_footfalls"
if (true) then { KJ_walkOn = false; ded disableAI "MOVE"; ded stop true }
triSimFrames 60
// -- 4. presence, at night ------------------------------------------------------------------
setDate [1985, 6, 10, 23, 30]
triSimFrames 200
// He is put back at the south edge of the town; the player stands 300 m north of him, in it.
if (true) then { ded setPos [5055, 3900, 0]; ded setDir 0; player setPos [5058, 4200, 0]; player setDir 180 }
triSimFrames 30
if (true) then { stAll = nearestObjects [getPos ded, [], 700]; stLamps = []; { if ((substr [typeOf _x, 0, 10]) == "StreetLamp") then { stLamps = stLamps + [_x] } } forEach stAll; stNear = 0; stFarL = 0; { if ((_x distance ded) < 190) then { stNear = stNear + 1 }; if ((_x distance ded) > 420) then { stFarL = stFarL + 1 } } forEach stLamps }
logInfo format ["KJSTEPTEST lamps before: %1 within 700 m, %2 within 190 m of him, %3 beyond 420 m", count stLamps, stNear, stFarL]
triAssertGt [stNear, 0]
// A view from 20 m behind the lamp nearest him, looking past it at him.
stLampView = { private ["_l", "_k", "_d", "_ux", "_uy", "_ex", "_ey", "_g"]; _l = getPos (_this select 0); _k = getPos ded; _d = sqrt (((_l select 0) - (_k select 0)) * ((_l select 0) - (_k select 0)) + ((_l select 1) - (_k select 1)) * ((_l select 1) - (_k select 1))); _ux = ((_l select 0) - (_k select 0)) / _d; _uy = ((_l select 1) - (_k select 1)) / _d; _ex = (_l select 0) + 20 * _ux; _ey = (_l select 1) + 20 * _uy; [_ex, _ey] call KJR_fnDry; _g = (getPosASL (6 call KJR_fnEmitter)) select 2; [_ex, _g + 1.8, _ey, 0 - _ux, 0.3, 0 - _uy] }
stView = [stLamps select 0] call stLampView
logInfo format ["KJSTEPTEST lamp view %1 for the lamp at %2 (%3 m from him)", stView, getPos (stLamps select 0), (stLamps select 0) distance ded]
triSetView stView
triSimFrames 6
triScreenshot "04_night_before_lamps_lit"
if (true) then { stP = count KJ_presenceLog; [ded] exec "kj\roar\presence.sqs"; stT0 = time }
triSimUntil { ((count KJ_darkLamps) >= stNear) or (time > stT0 + 12) }
triSimUntil { time > stT0 + 12 }
if (true) then { stOff = 0; stOffFar = 0; stWrong = 0; { if ((lightIsOn _x) == "OFF") then { stOff = stOff + 1; if ((_x distance ded) > 420) then { stOffFar = stOffFar + 1 } } } forEach stLamps; { if ((_x distance ded) > 215) then { stWrong = stWrong + 1 } } forEach KJ_darkLamps }
logInfo format ["KJSTEPTEST lamps after 12 s: %1 put out by him (%2 of them further than 215 m), %3 read OFF now, %4 of those beyond 420 m", count KJ_darkLamps, stWrong, stOff, stOffFar]
triAssertGe [count KJ_darkLamps, stNear]
triAssertEq [stWrong, 0]
triAssertEq [stOffFar, 0]
triSetView stView
triSimFrames 6
triScreenshot "05_night_after_lamps_out"
// The drone (player 300 m off, inside 400), the radio and the birds.
stHas = { private ["_n"]; _n = 0; { if ((_x select 0) == _this) then { _n = _n + 1 } } forEach KJ_presenceLog; _n }
triSimUntil { (("STATIC" call stHas) > 0) or (time > stT0 + 30) }
logInfo format ["KJSTEPTEST presence log: %1", KJ_presenceLog]
triAssertGt ["DRONE" call stHas, 0]
triAssertGt ["DARK" call stHas, 0]
triAssertGt ["STATIC" call stHas, 0]
triAssertGt ["BIRDS" call stHas, 0]
// The birds: all of them made, and going away from him.
if (true) then { stB = + KJ_birds; stB0 = []; { stB0 = stB0 + [_x distance ded] } forEach stB; stT1 = time }
triAssertEq [count stB, 9]
triSimUntil { time > stT1 + 6 }
if (true) then { stAway = 0; stI = 0; { if ((_x distance ded) > ((stB0 select stI) + 20)) then { stAway = stAway + 1 }; stI = stI + 1 } forEach stB }
logInfo format ["KJSTEPTEST birds: %1 made, %2 are 20 m or more further from him after 6 s; first went from %3 to %4 m", count stB, stAway, stB0 select 0, (stB select 0) distance ded]
triAssertGe [stAway, 7]
// The drone is restarted while the player stays near.
triSimUntil { (("DRONE" call stHas) > 1) or (time > stT0 + 40) }
triAssertGt ["DRONE" call stHas, 1]
if (true) then { KJ_presenceOn = false }
logInfo format ["KJSTEPTEST end: time %1, errors %2", time, triErrorCount]
triEndTest
