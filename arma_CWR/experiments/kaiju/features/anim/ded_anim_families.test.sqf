// ============================================================================
//  Dedushka stance families: the low and crouch copies of the loops, entered without any gunfire
//  Written by gen_ded_anim_tests.py (edit that, not this file).
//  Needs @kj_anim (features/anim/build-kj-anim.ps1). Quiet: nobody is armed,
//  nobody fires, --nosound. DEDA / DEDS / DEDW / DEDG / DEDF lines in
//  game_stdout.log are the measurements analyze_anim.py reads.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.35
0 setFog 0
setViewDistance 2500
showCinemaBorder false
daC = createCenter civilian
daG = createGroup civilian
ded = objNull
player setPos [4740, 11260, 0]
daAt = [4800, 11000, 0]
"UDDedushka" createUnit [daAt, daG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushka"]
triAssert [alive ded]
daAnim = 0
ded addEventHandler ["AnimChanged", { daAnim = daAnim + 1; logInfo format ["DEDA anim %1 %2", time, _this select 1] }]
daHold = { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; daG setBehaviour "CARELESS"; ded setUnitPos "UP"; ded stop true; ded setDir 180; ded setPos daAt }
call daHold
ded allowDammage false
// daView: [bearing from him (0 = north of him), distance, eye height above the
// local ground, aim height above his feet] -> a triSetView array.
daProbe = "HeliHEmpty" createVehicle [4800, 11300, 0]
daView = { private ["_b", "_d", "_e", "_a", "_p", "_cx", "_cz", "_g"]; _b = _this select 0; _d = _this select 1; _e = _this select 2; _a = _this select 3; _p = getPosASL ded; _cx = (_p select 0) + _d * sin _b; _cz = (_p select 1) + _d * cos _b; daProbe setPos [_cx, _cz, 0]; _g = (getPosASL daProbe) select 2; [_cx, _g + _e, _cz, (_p select 0) - _cx, ((_p select 2) + _a) - (_g + _e), (_p select 1) - _cz] }
daCam = "camera" camCreate [4800, 10800, 30]
daCam cameraEffect ["internal", "back"]
daFov = { daCam camSetTarget [4800, 11000, 30]; daCam camSetFov _this; daCam camCommit 0 }
0.7 call daFov
triSimFrames 200
triAssertEq [getMove ded, "dedidle"]
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 6
triScreenshot "00_warmup_discard"
// ---- "down": the stance the AI wants under fire. He must stay upright, in the low copy of the idle.
if (true) then { daT0 = time; daN0 = daAnim; ded setUnitPos "DOWN" }
triSimUntil { time > daT0 + 3.0 }
logInfo format ["DEDA low idle %1 %2", time, getMove ded]
triAssertEq [getMove ded, "dedidlelow"]
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "low_1_idle"
logInfo format ["DEDS low_1_idle %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
// ---- walking in the low family
if (true) then { daT0 = time; daP0 = getPosASL ded; ded stop false; ded enableAI "MOVE"; ded doMove [4800, 10700, 0] }
triSimUntil { time > daT0 + 5.0 }
logInfo format ["DEDA low walk %1 %2 moved %3", time, getMove ded, (getPosASL ded) distance daP0]
triAssertEq [getMove ded, "dedwalklow"]
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "low_2_walk"
logInfo format ["DEDS low_2_walk %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
// ---- stance change in mid stride: back to the upright family
if (true) then { daT0 = time; ded setUnitPos "UP" }
triSimUntil { time > daT0 + 3.0 }
logInfo format ["DEDA up walk %1 %2 moved %3", time, getMove ded, (getPosASL ded) distance daP0]
triAssertEq [getMove ded, "dedwalk"]
triAssert [((getPosASL ded) distance daP0) > 8]
// ---- stop, then COMBAT with free stance: the AI asks for crouch or low; he must settle in one of the idle copies
if (true) then { daT0 = time; doStop ded; ded doMove (getPos ded) }
triSimUntil { time > daT0 + 4.0 }
triAssertEq [getMove ded, "dedidle"]
if (true) then { daT0 = time; ded setUnitPos "AUTO"; daG setBehaviour "COMBAT"; daG setCombatMode "GREEN" }
triSimUntil { time > daT0 + 8.0 }
logInfo format ["DEDA combat auto %1 %2", time, getMove ded]
triAssert [(getMove ded) in ["dedidle", "dedidlelow", "dedidlecrouch"]]
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "combat_1_idle"
logInfo format ["DEDS combat_1_idle %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
// ---- an act from whichever family he is in, and back
if (true) then { daT0 = time; ded playMove "DedFlinch" }
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedflinch"]
triSimUntil { time > daT0 + 6.60 }
logInfo format ["DEDA after flinch %1 %2", time, getMove ded]
triAssert [(getMove ded) in ["dedidle", "dedidlelow", "dedidlecrouch"]]
// ---- a turn in the low family
if (true) then { daT0 = time; daD0 = getDir ded; daG setBehaviour "CARELESS"; ded setUnitPos "DOWN"; ded doWatch [4800, 11400, 0] }
triSimUntil { time > daT0 + 6.0 }
logInfo format ["DEDA low turn %1 %2 dir %3 from %4", time, getMove ded, getDir ded, daD0]
triAssert [(getMove ded) in ["dedturnllow", "dedturnrlow"]]
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "low_3_turn"
logInfo format ["DEDS low_3_turn %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 20.0 }
logInfo format ["DEDA low turn done %1 %2 dir %3 animChanges %4", time, getMove ded, getDir ded, daAnim - daN0]
triAssert [(cos ((getDir ded) - daD0)) < -0.9]
// A stance loop shows as hundreds of move changes; the whole test needs about twenty.
triAssert [(daAnim - daN0) < 60]
triAssert [alive ded]
triEndTest
