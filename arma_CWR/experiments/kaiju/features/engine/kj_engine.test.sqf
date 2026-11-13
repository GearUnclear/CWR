// ============================================================================
//  Engine support for the giant creature (engine/Poseidon/Game/Kaiju), proven
//  on the proxy UDDedushka at the photo site of the pipeline (south edge of
//  Morton, Everon; sea to the south). Every block asserts on state and takes
//  the few frames that show the effect. API contract: API.md beside this file.
//  Quiet: nobody is armed, nobody fires. Damage is scripted (kjDamage).
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [5060, 4120, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.2
0 setFog 0
// the stock view distance: objects stop at 600 m, the far plane is at 900 m
setViewDistance 900
showCinemaBorder false
kjC = createCenter civilian
kjG = createGroup civilian
ded = objNull
kjAt = [5055, 3870, 0]
"UDDedushka" createUnit [kjAt, kjG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
kjFails = []
// soft check: [condition, label]; failures are collected and asserted once at the end
kjChk = { if (!(_this select 0)) then { kjFails = kjFails + [_this select 1]; logInfo format ["KJENGINE CHECK FAILED: %1", _this select 1] } }
kjHold = { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; ded setBehaviour "CARELESS"; ded stop true; ded setDir 180; ded setPos kjAt }
call kjHold
kjProbe = "HeliHEmpty" createVehicle [5055, 4200, 0]
kjCam = "camera" camCreate [5055, 3700, 30]
kjCam cameraEffect ["internal", "back"]
kjFov = { kjCam camSetTarget [5055, 3900, 30]; kjCam camSetFov _this; kjCam camCommit 0 }
0.7 call kjFov
// kjView: [bearing from the creature, distance, eye height, aim height] -> triSetView argument
kjView = { private ["_b", "_d", "_e", "_a", "_p", "_cx", "_cz", "_g"]; _b = _this select 0; _d = _this select 1; _e = _this select 2; _a = _this select 3; _p = getPosASL ded; _cx = (_p select 0) + _d * sin _b; _cz = (_p select 1) + _d * cos _b; kjProbe setPos [_cx, _cz, 0]; _g = (getPosASL kjProbe) select 2; [_cx, _g + _e, _cz, (_p select 0) - _cx, ((_p select 2) + _a) - (_g + _e), (_p select 1) - _cz] }
// the same camera, looking the other way
kjViewAway = { private ["_v"]; _v = _this call kjView; [_v select 0, _v select 1, _v select 2, 0 - (_v select 3), 0, 0 - (_v select 5)] }
triSimFrames 120
triSetView ([180, 60, 1.7, 20] call kjView)
triSimFrames 6
triScreenshot "00_warmup_discard"

// -- the API is there --------------------------------------------------------
[(format ["%1", kjVersion]) == (format ["%1", 2]), format ["L43 kjVersion == 2 (got %1; 2 since the finishing round)", kjVersion]] call kjChk
[(format ["%1", kjGet [ded, "farDraw"]]) == (format ["%1", 0]), format ["L44 kjGet [ded, 'farDraw'] == 0 (got %1)", kjGet [ded, "farDraw"]]] call kjChk
[(format ["%1", kjShakeGain]) == (format ["%1", 0]), format ["L45 kjShakeGain == 0 (got %1)", kjShakeGain]] call kjChk
[!(kjSet [ded, "noSuchTrait", 1]), "L46 !(kjSet [ded, 'noSuchTrait', 1])"] call kjChk

// the detection idiom of API.md (on the stock binary both sides print the same)
[(format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_]), "detection idiom says native"] call kjChk

// -- lamp commands on something that is not a lamp ---------------------------
// Stock: lightIsOn of a soldier answered "OFF", switchLight wrote lamp state
// into him and the game died a frame later (found by the roar feature).
kjLampAnswer = lightIsOn player
player switchLight "OFF"
ded switchLight "OFF"
triSimFrames 10
logInfo format ["KJENGINE lamp: lightIsOn player = %1, lightIsOn ded = %2", kjLampAnswer, lightIsOn ded]
[kjLampAnswer == "ERROR", "lightIsOn of a soldier answers ERROR"] call kjChk
[(lightIsOn ded) == "ERROR", "lightIsOn of the creature answers ERROR"] call kjChk
[alive player && alive ded, "switchLight on non-lamps leaves them alive"] call kjChk

// -- nearMip trait (texture levels from the nearest point of the body) -------
// Only that the switch is there and reads back; what it does to the picture is
// for the texture study to measure (texres_charts on this binary).
[kjSet [ded, "nearMip", 1], "kjSet nearMip"] call kjChk
[(kjGet [ded, "nearMip"]) == 1, "kjGet nearMip reads 1"] call kjChk

// -- memory points -----------------------------------------------------------
// The proxy has the stock foot points; an offset form works on any model.
kjPt = kjPoint [ded, [0, 0, 10]]
kjPa = getPosASL ded
logInfo format ["KJENGINE point: offset %1 ded asl %2 stopaL %3 nosuch %4", kjPt, kjPa, kjPoint [ded, "stopaL"], kjPoint [ded, "noSuchPoint"]]
[(count kjPt) == 3, "kjPoint offset returns a position"] call kjChk
[abs (((kjPt select 2) - (kjPa select 2)) - 10) < 0.5, "kjPoint offset is 10 m above the feet"] call kjChk
[(count (kjPoint [ded, "noSuchPoint"])) == 0, "kjPoint unknown point returns []"] call kjChk

// -- far draw ----------------------------------------------------------------
// Off: at 2 km the creature is not drawn (long lens so it would fill the frame).
0.1 call kjFov
triSetView ([180, 2000, 1.7, 20] call kjView)
triSimFrames 6
[(format ["%1", (kjFarState ded) select 0]) == (format ["%1", 0]), format ["L53 (kjFarState ded) select 0 == 0 (got %1)", (kjFarState ded) select 0]] call kjChk
triScreenshot "f00_far_off_2km_zoom"
// On, out to 6 km.
[kjSet [ded, "farDraw", 6000], "L56 kjSet [ded, 'farDraw', 6000]"] call kjChk
[(format ["%1", kjGet [ded, "farDraw"]]) == (format ["%1", 6000]), format ["L57 kjGet [ded, 'farDraw'] == 6000 (got %1)", kjGet [ded, "farDraw"]]] call kjChk
triSimFrames 6
kjFs = kjFarState ded
logInfo format ["KJENGINE far 2km: %1", kjFs]
[(format ["%1", kjFs select 0]) == (format ["%1", 2]), format ["L61 kjFs select 0 == 2 (got %1)", kjFs select 0]] call kjChk
[(kjFs select 4) > 0, "L62 (kjFs select 4) > 0"] call kjChk
[(kjFs select 2) > 0.1, "L63 (kjFs select 2) > 0.1"] call kjChk
triScreenshot "f01_far_on_2km_zoom"
triSetView ([180, 4000, 1.7, 20] call kjView)
triSimFrames 6
[(format ["%1", (kjFarState ded) select 0]) == (format ["%1", 2]), format ["L67 (kjFarState ded) select 0 == 2 (got %1)", (kjFarState ded) select 0]] call kjChk
triScreenshot "f02_far_on_4km_zoom"
// Normal lens: the horizon view a player gets.
0.7 call kjFov
triSetView ([180, 1500, 1.7, 20] call kjView)
triSimFrames 6
[(format ["%1", (kjFarState ded) select 0]) == (format ["%1", 2]), format ["L73 (kjFarState ded) select 0 == 2 (got %1)", (kjFarState ded) select 0]] call kjChk
triScreenshot "f03_far_on_1500m"
triSetView ([180, 3000, 1.7, 20] call kjView)
triSimFrames 6
triScreenshot "f04_far_on_3km"
// Inside the far plane but past the object distance: the normal pass, exempt.
triSetView ([180, 720, 1.7, 20] call kjView)
triSimFrames 6
kjFs = kjFarState ded
logInfo format ["KJENGINE far 720m: %1", kjFs]
[(format ["%1", kjFs select 0]) == (format ["%1", 1]), format ["L83 kjFs select 0 == 1 (got %1)", kjFs select 0]] call kjChk
triScreenshot "f05_far_on_720m"
// Looking away from it: the far pass has nothing to draw.
triSetView ([180, 2000, 1.7, 20] call kjViewAway)
triSimFrames 6
[((kjFarState ded) select 0) != 2, "L88 ((kjFarState ded) select 0) != 2"] call kjChk
// Past its limit: stock rules again.
[kjSet [ded, "farDraw", 1500], "L90 kjSet [ded, 'farDraw', 1500]"] call kjChk
triSetView ([180, 2000, 1.7, 20] call kjView)
triSimFrames 6
[(format ["%1", (kjFarState ded) select 0]) == (format ["%1", 0]), format ["L93 (kjFarState ded) select 0 == 0 (got %1)", (kjFarState ded) select 0]] call kjChk
[kjSet [ded, "farDraw", 6000], "L94 kjSet [ded, 'farDraw', 6000]"] call kjChk

// -- camera shake --------------------------------------------------------------
triSetView ([180, 60, 1.7, 20] call kjView)
triSimFrames 6
triScreenshot "s00_shake_before"
[kjShake [ded, 2, 100, 400, 3], "L100 kjShake [ded, 2, 100, 400, 3]"] call kjChk
triSimFrames 8
kjSg = kjShakeGain
logInfo format ["KJENGINE shake gain after 8 frames: %1", kjSg]
[kjSg > 0.5, "L104 kjSg > 0.5"] call kjChk
triScreenshot "s01_shake_during"
// out of range: nothing felt 2 km away
triSetView ([180, 2000, 1.7, 20] call kjView)
triSimFrames 3
[(format ["%1", kjShakeGain]) == (format ["%1", 0]), format ["L109 kjShakeGain == 0 (got %1)", kjShakeGain]] call kjChk
if (true) then { kjT0 = time }
triSimUntil { time > kjT0 + 3.5 }
triSetView ([180, 60, 1.7, 20] call kjView)
triSimFrames 3
[(format ["%1", kjShakeGain]) == (format ["%1", 0]), format ["L114 kjShakeGain == 0 (got %1)", kjShakeGain]] call kjChk
// malformed argument: quiet false, not a script error
[!(kjShake [ded, 1]), "L116 !(kjShake [ded, 1])"] call kjChk

// -- steer, footfalls ----------------------------------------------------------
// The AI has MOVE disabled and is stopped; the steer walks it anyway, due east.
if (true) then { kjP0 = getPosASL ded; kjT0 = time; kjS0 = (kjSteps ded) select 0 }
[kjSteer [ded, [5115, 3870, 0], "walk"], "L121 kjSteer [ded, [5115, 3870, 0], 'walk']"] call kjChk
[(kjSteerState ded) select 0, "L122 (kjSteerState ded) select 0"] call kjChk
triSimUntil { time > kjT0 + 14 }
if (true) then { kjP1 = getPosASL ded; kjMoved = (kjP1 select 0) - (kjP0 select 0); kjDrift = abs ((kjP1 select 1) - (kjP0 select 1)); kjS1 = (kjSteps ded) select 0 }
logInfo format ["KJENGINE steer: moved east %1 m, drift %2 m, dir %3, move %4, steps %5, state %6", kjMoved, kjDrift, getDir ded, getMove ded, kjS1 - kjS0, kjSteerState ded]
[kjMoved > 8, "L126 kjMoved > 8"] call kjChk
[kjDrift < 4, "L127 kjDrift < 4"] call kjChk
[(kjS1 - kjS0) > 0, "L128 (kjS1 - kjS0) > 0"] call kjChk
triSetView ([180, 75, 1.7, 18] call kjView)
triSimFrames 3
triScreenshot "w01_steer_walk"
// footfall tremor: each step shakes a camera at its feet
[kjSet [ded, "stepShake", 1.5], "L133 kjSet [ded, 'stepShake', 1.5]"] call kjChk
triSetView ([180, 20, 1.7, 20] call kjView)
if (true) then { kjS0 = (kjSteps ded) select 0; kjPeak = 0; kjT0 = time }
triSimUntil { if (kjShakeGain > kjPeak) then { kjPeak = kjShakeGain }; (((kjSteps ded) select 0) > kjS0 + 1) || (time > kjT0 + 12) }
logInfo format ["KJENGINE step shake: peak gain %1, steps %2", kjPeak, ((kjSteps ded) select 0) - kjS0]
[kjPeak > 0.3, "L138 kjPeak > 0.3"] call kjChk
[kjSet [ded, "stepShake", 0], "L139 kjSet [ded, 'stepShake', 0]"] call kjChk
// arrival, then release
triSimUntil { ((kjSteerState ded) select 1) || (time > kjT0 + 40) }
logInfo format ["KJENGINE steer arrive: state %1 pos %2", kjSteerState ded, getPosASL ded]
[(kjSteerState ded) select 1, "L143 (kjSteerState ded) select 1"] call kjChk
[kjSteerStop ded, "L144 kjSteerStop ded"] call kjChk
[!((kjSteerState ded) select 0), "L145 !((kjSteerState ded) select 0)"] call kjChk
call kjHold
triSimFrames 30

// -- damage ceiling, hit reports, knockback ------------------------------------
[kjSet [ded, "hitReport", 1], "L150 kjSet [ded, 'hitReport', 1]"] call kjChk
[kjSet [ded, "damageCeiling", 0.4], "L151 kjSet [ded, 'damageCeiling', 0.4]"] call kjChk
if (true) then { ded setDammage 0; kjD0 = getDammage ded }
[kjDamage [ded, player, 0, 4000, 40], "L153 kjDamage [ded, player, 0, 4000, 40]"] call kjChk
[kjDamage [ded, player, 0, 4000, 40], "L154 kjDamage [ded, player, 0, 4000, 40]"] call kjChk
[kjDamage [ded, player, 0, 400000, 40], "L155 kjDamage [ded, player, 0, 400000, 40]"] call kjChk
triSimFrames 5
kjH = kjHits ded
logInfo format ["KJENGINE damage: before %1 after %2 alive %3 hits %4", kjD0, getDammage ded, alive ded, kjH]
[(count (kjH select 0)) == 6, "hit record has six fields"] call kjChk
[(((kjH select 0) select 5) distance ded) < 60, "hit record world position is on the creature"] call kjChk
[alive ded, "L159 alive ded"] call kjChk
[(getDammage ded) <= 0.401, "L160 (getDammage ded) <= 0.401"] call kjChk
[(getDammage ded) > 0.2, "L161 (getDammage ded) > 0.2"] call kjChk
[(format ["%1", count kjH]) == (format ["%1", 3]), format ["L162 count kjH == 3 (got %1)", count kjH]] call kjChk
[((kjH select 0) select 0) == player, "L163 ((kjH select 0) select 0) == player"] call kjChk
[(format ["%1", count (kjHits ded)]) == (format ["%1", 0]), format ["L164 count (kjHits ded) == 0 (got %1)", count (kjHits ded)]] call kjChk
[kjSet [ded, "damageCeiling", -1], "L165 kjSet [ded, 'damageCeiling', -1]"] call kjChk
ded setDammage 0
// knockback: an empty tank thrown without a shot
kjTank = "T80" createVehicle [5140, 3840, 0]
triSimFrames 90
if (true) then { kjP0 = getPosASL kjTank; kjT0 = time }
[kjImpulse [kjTank, [8, 0, 6]], "L171 kjImpulse [kjTank, [8, 0, 6]]"] call kjChk
triSimUntil { time > kjT0 + 2 }
logInfo format ["KJENGINE knockback: tank moved %1 m, from %2 to %3", kjTank distance kjP0, kjP0, getPosASL kjTank]
[(kjTank distance kjP0) > 1.5, "L174 (kjTank distance kjP0) > 1.5"] call kjChk

// -- wading --------------------------------------------------------------------
// 7 m of water at [5000, 3770]. The drowning rule (more than 1.2 m hurts a Man)
// is shown on two plain civilians, because the proxy is so large and so
// armoured that the per-frame drowning damage is under the engine's own noise
// floor for him either way. One civilian wades, the other does not.
kjWG = createGroup civilian
kjWa = objNull
kjWb = objNull
"Civilian" createUnit [[5003, 3770, 0], kjWG, "kjWa = this", 0.5, "PRIVATE"]
"Civilian" createUnit [[4997, 3770, 0], kjWG, "kjWb = this", 0.5, "PRIVATE"]
triSimFrames 10
// held where they are: left alone they walk out of the water
if (true) then { { _x disableAI "MOVE"; _x stop true; _x setBehaviour "CARELESS" } forEach [kjWa, kjWb]; kjWa setPos [5003, 3770, 0]; kjWb setPos [4997, 3770, 0] }
[!(isNull kjWa) && !(isNull kjWb), "wading civilians exist"] call kjChk
[kjSet [kjWa, "wadeDepth", 30], "kjSet wadeDepth on a civilian"] call kjChk
// The stock drowning damage is 0.05 x frame time x 2 a frame, and the damage
// code drops anything under 0.001: at a high frame rate a man never drowns.
// Four times time acceleration makes the frame step large enough to count.
setAccTime 4
if (true) then { kjWa setDammage 0; kjWb setDammage 0; kjT0 = time }
triSimUntil { ((getDammage kjWb) > 0.03) || (time > kjT0 + 60) }
setAccTime 1
logInfo format ["KJENGINE wading: after %1 s, wader asl %2 dammage %3, stock asl %4 dammage %5", time - kjT0, (getPosASL kjWa) select 2, getDammage kjWa, (getPosASL kjWb) select 2, getDammage kjWb]
[(getDammage kjWb) > 0.03, "a stock man takes drowning damage in 7 m"] call kjChk
[(getDammage kjWa) == 0, "a wading man takes no damage in 7 m"] call kjChk
[alive kjWa, "the wading man is alive"] call kjChk
deleteVehicle kjWa
deleteVehicle kjWb
// the creature itself: stands in 7 m, then is steered north out of the sea
[kjSet [ded, "wadeDepth", 30], "kjSet wadeDepth on the creature"] call kjChk
ded setPos [5000, 3770, 0]
ded setDir 180
if (true) then { ded setDammage 0; kjT0 = time }
triSimUntil { time > kjT0 + 6 }
logInfo format ["KJENGINE wading creature: feet asl %1 (sea surface about 2.7), dammage %2, alive %3", (getPosASL ded) select 2, getDammage ded, alive ded]
[(getDammage ded) == 0, "the creature takes no damage in 7 m"] call kjChk
triSetView ([75, 60, 1.7, 12] call kjView)
triSimFrames 6
triScreenshot "x01_wading_7m"
[kjSteer [ded, [5000, 3840, 0], "walk"], "kjSteer out of the sea"] call kjChk
if (true) then { kjT0 = time; kjP0 = getPosASL ded }
triSimUntil { time > kjT0 + 14 }
logInfo format ["KJENGINE wade walk: from %1 to %2, dammage %3, move %4", kjP0, getPosASL ded, getDammage ded, getMove ded]
[((getPosASL ded) select 1) > ((kjP0 select 1) + 8), "the creature walks out of the sea"] call kjChk
[(getDammage ded) == 0, "no damage on the way out"] call kjChk
[kjSteerStop ded, "kjSteerStop"] call kjChk
call kjHold
triSimFrames 30

// -- obstacle-blind planning ---------------------------------------------------
// The AI is given a doMove through the town north of him (houses on the line).
// Blind: the planner goes straight. kjDev is the furthest he strays from the line.
kjTrack = { private ["_d"]; _d = abs (((getPosASL ded) select 0) - 5055); if (_d > kjDev) then { kjDev = _d }; time > kjT0 + 30 }
[kjSet [ded, "obstacleBlind", 1], "kjSet obstacleBlind"] call kjChk
if (true) then { ded setPos kjAt; ded setDir 0; ded stop false; ded enableAI "MOVE"; ded setSpeedMode "FULL"; kjDev = 0; kjT0 = time; ded doMove [5055, 4040, 0] }
triSimUntil { call kjTrack }
kjBlindDev = kjDev
kjBlindNorth = ((getPosASL ded) select 1) - 3870
logInfo format ["KJENGINE blind doMove: went north %1 m, strayed %2 m, pos %3", kjBlindNorth, kjBlindDev, getPosASL ded]
[kjBlindNorth > 20, "blind doMove walks north"] call kjChk
[kjBlindDev < 6, "blind doMove keeps to the straight line"] call kjChk
[kjSet [ded, "obstacleBlind", 0], "kjSet obstacleBlind off"] call kjChk
if (true) then { ded setPos kjAt; ded setDir 0; kjDev = 0; kjT0 = time; ded doMove [5055, 4041, 0] }
triSimUntil { call kjTrack }
logInfo format ["KJENGINE stock doMove: went north %1 m, strayed %2 m, pos %3", ((getPosASL ded) select 1) - 3870, kjDev, getPosASL ded]
call kjHold
triSimFrames 30

// -- roar as a world event -------------------------------------------------------
// No sound in this run (--nosound); the reveal and the flight are what is asserted.
kjCG = createGroup civilian
kjCiv = objNull
"Civilian" createUnit [[5055, 3990, 0], kjCG, "kjCiv = this", 0.5, "PRIVATE"]
triSimFrames 30
[!(isNull kjCiv), "L213 !(isNull kjCiv)"] call kjChk
kjK0 = kjCiv knowsAbout ded
kjR = kjRoar [ded, "", 1000, 200, 30]
triSimFrames 60
logInfo format ["KJENGINE roar: [revealed, fled] %1, civilian knowsAbout %2 -> %3, fleeing %4", kjR, kjK0, kjCiv knowsAbout ded, fleeing kjCiv]
[(kjR select 0) >= 1, "L218 (kjR select 0) >= 1"] call kjChk
[(kjCiv knowsAbout ded) > 3, "L219 (kjCiv knowsAbout ded) > 3"] call kjChk
[(kjR select 1) >= 1, "L220 (kjR select 1) >= 1"] call kjChk
[fleeing kjCiv, "L221 fleeing kjCiv"] call kjChk

// -- light and emitter, at night -----------------------------------------------
setDate [1985, 6, 10, 23, 30]
triSimFrames 300
triSetView ([150, 70, 1.7, 14] call kjView)
triSimFrames 6
triScreenshot "n00_night_no_light"
[(format ["%1", kjLightCount ded]) == (format ["%1", 0]), format ["L229 kjLightCount ded == 0 (got %1)", kjLightCount ded]] call kjChk
[kjLight [ded, "furnace", [0, 6, 24], [1, 0.42, 0.1], 1.2], "L230 kjLight [ded, 'furnace', [0, 6, 24], [1, 0.42, 0.1], 1.2]"] call kjChk
[(format ["%1", kjLightCount ded]) == (format ["%1", 1]), format ["L231 kjLightCount ded == 1 (got %1)", kjLightCount ded]] call kjChk
triSimFrames 10
triScreenshot "n01_night_furnace"
// the beam charge: the same slot, brighter
[kjLight [ded, "furnace", [0, 6, 24], [1, 0.55, 0.2], 4], "L235 kjLight [ded, 'furnace', [0, 6, 24], [1, 0.55, 0.2], 4]"] call kjChk
[(format ["%1", kjLightCount ded]) == (format ["%1", 1]), format ["L236 kjLightCount ded == 1 (got %1)", kjLightCount ded]] call kjChk
triSimFrames 10
triScreenshot "n02_night_furnace_charge"
triSetView ([350, 230, 1.7, 22] call kjView)
triSimFrames 6
triScreenshot "n03_night_furnace_from_town"
triSetView ([200, 140, 70, 2] call kjView)
triSimFrames 6
triScreenshot "n04_night_furnace_from_above"
// emitter: smoke from the chest
[kjEmitter [ded, "smoke", ["cl_basic", "", "Billboard", 1, 6, [0, 6, 26], [0, 0, 3], 0, 1.275, 1, 0, [6, 14], [[1, 0.5, 0.2, 0.5], [0.4, 0.4, 0.4, 0.3], [0.3, 0.3, 0.3, 0]], [0], 0.5, 1, "", "", ded], 12], "L246 kjEmitter [ded, 'smoke', ['cl_basic', '', 'Billboard', 1, 6, [0, 6, 26], [0, 0, 3], 0, 1.275, 1, 0, [6, 14], [[1, 0.5, 0.2, 0.5], [0.4, 0.4, 0.4, 0.3], [0.3, 0.3, 0.3, 0]], [0], 0.5, 1, '', '', ded], 12]"] call kjChk
triSimFrames 240
triSetView ([150, 70, 1.7, 14] call kjView)
triSimFrames 6
triScreenshot "n05_night_furnace_smoke"
[kjEmitter [ded, "smoke", [], 0], "L251 kjEmitter [ded, 'smoke', [], 0]"] call kjChk
// far at night: the lit chest region on the horizon
triSetView ([180, 2000, 1.7, 20] call kjView)
triSimFrames 6
triScreenshot "n06_night_far_2km"
[kjLight [ded, "furnace", [0, 6, 24], [1, 0.5, 0.2], 0], "L256 kjLight [ded, 'furnace', [0, 6, 24], [1, 0.5, 0.2], 0]"] call kjChk
[(format ["%1", kjLightCount ded]) == (format ["%1", 0]), format ["L257 kjLightCount ded == 0 (got %1)", kjLightCount ded]] call kjChk

// -- clean removal ---------------------------------------------------------------
[kjClear ded, "L260 kjClear ded"] call kjChk
[(format ["%1", kjGet [ded, "farDraw"]]) == (format ["%1", 0]), format ["L261 kjGet [ded, 'farDraw'] == 0 (got %1)", kjGet [ded, "farDraw"]]] call kjChk
[alive ded, "L262 alive ded"] call kjChk
logInfo format ["KJENGINE end: %1 failed checks %2", count kjFails, kjFails]
triAssertEq [format ["%1", kjFails], "[]"]
triEndTest
