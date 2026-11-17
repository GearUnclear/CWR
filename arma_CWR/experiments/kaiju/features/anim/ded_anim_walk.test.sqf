// ============================================================================
//  Dedushka walk: speed, foot slide, footfalls and the turn on the spot, from a fixed side camera
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
player setPos [4680, 11400, 0]
daAt = [4766, 11200, 0]
"UDDedushka" createUnit [daAt, daG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
triAssertEq [typeOf ded, "UDDedushka"]
triAssert [alive ded]
daAnim = 0
ded addEventHandler ["AnimChanged", { daAnim = daAnim + 1; logInfo format ["DEDA anim %1 %2", time, _this select 1] }]
daHold = { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; daG setBehaviour "CARELESS"; ded setUnitPos "UP"; ded stop true; ded setDir 90; ded setPos daAt }
call daHold
ded allowDammage false
// daView: [bearing from him (0 = north of him), distance, eye height above the
// local ground, aim height above his feet] -> a triSetView array.
daProbe = "HeliHEmpty" createVehicle [4800, 11500, 0]
daView = { private ["_b", "_d", "_e", "_a", "_p", "_cx", "_cz", "_g"]; _b = _this select 0; _d = _this select 1; _e = _this select 2; _a = _this select 3; _p = getPosASL ded; _cx = (_p select 0) + _d * sin _b; _cz = (_p select 1) + _d * cos _b; daProbe setPos [_cx, _cz, 0]; _g = (getPosASL daProbe) select 2; [_cx, _g + _e, _cz, (_p select 0) - _cx, ((_p select 2) + _a) - (_g + _e), (_p select 1) - _cz] }
daCam = "camera" camCreate [4800, 10850, 30]
daCam cameraEffect ["internal", "back"]
daFov = { daCam camSetTarget [4800, 11200, 30]; daCam camSetFov _this; daCam camCommit 0 }
0.105 call daFov
call preprocessFile "kj\anim\ded_anim_timing.sqf"
triAssert [KJ_animLoaded]
// ground heights along the track and under the camera
daRow = []; daGx = 4740; while { daGx <= 4860 } do { daProbe setPos [daGx, 11200, 0]; daRow = daRow + [(getPosASL daProbe) select 2]; daGx = daGx + 5 }; logInfo format ["DEDG track %1 %2 %3", 11200, 4740, daRow]
triSimFrames 200
triAssertEq [getMove ded, "dedidle"]
daProbe setPos [4800, 10900, 0]
daCamY = ((getPosASL daProbe) select 2) + 4.0
daFix = [4800, daCamY, 10900, 0, ((getPosASL ded) select 2) + 14 - daCamY, 300]
logInfo format ["DEDC cam %1 fov 0.105 dist 300 mid [4800,11200] ded asl %2", daFix, getPosASL ded]
triSetView daFix
triSimFrames 6
triScreenshot "00_warmup_discard"
ded setPos [5200, 11500, 0]
triSimFrames 8
triScreenshot "bg"
logInfo format ["DEDS bg %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (true) then { ded setPos [4780, 11200, 0]; ded setDir 90 }
triSimFrames 30
triScreenshot "cal_a"
logInfo format ["DEDS cal_a %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (true) then { ded setPos [4820, 11200, 0]; ded setDir 90 }
triSimFrames 30
triScreenshot "cal_b"
logInfo format ["DEDS cal_b %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
call daHold
triSimFrames 60
[ded] exec "kj\anim\footfalls.sqs"
DEDW_ON = true
DEDW_QUIT = false
[ded, 0.05] exec "ded_watch.sqs"
daFalls = 0
triSimFrames 20
// ---- the walk: the AI takes him east at his full rate (the goal is far), then he is told to stop
if (true) then { daT0 = time; daP0 = getPosASL ded; ded stop false; ded enableAI "MOVE"; ded doMove [5060, 11200, 0] }
logInfo format ["DEDA walk start %1 %2", time, getPosASL ded]
triSimUntil { time >= daT0 + 0.00 }
triScreenshot "w_000"
logInfo format ["DEDS w_000 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.25 }
triScreenshot "w_001"
logInfo format ["DEDS w_001 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.50 }
triScreenshot "w_002"
logInfo format ["DEDS w_002 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.75 }
triScreenshot "w_003"
logInfo format ["DEDS w_003 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.00 }
triScreenshot "w_004"
logInfo format ["DEDS w_004 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.25 }
triScreenshot "w_005"
logInfo format ["DEDS w_005 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.50 }
triScreenshot "w_006"
logInfo format ["DEDS w_006 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.75 }
triScreenshot "w_007"
logInfo format ["DEDS w_007 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.00 }
triScreenshot "w_008"
logInfo format ["DEDS w_008 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.25 }
triScreenshot "w_009"
logInfo format ["DEDS w_009 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.50 }
triScreenshot "w_010"
logInfo format ["DEDS w_010 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.75 }
triScreenshot "w_011"
logInfo format ["DEDS w_011 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.00 }
triScreenshot "w_012"
logInfo format ["DEDS w_012 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.25 }
triScreenshot "w_013"
logInfo format ["DEDS w_013 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.50 }
triScreenshot "w_014"
logInfo format ["DEDS w_014 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.75 }
triScreenshot "w_015"
logInfo format ["DEDS w_015 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.00 }
triScreenshot "w_016"
logInfo format ["DEDS w_016 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.25 }
triScreenshot "w_017"
logInfo format ["DEDS w_017 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.50 }
triScreenshot "w_018"
logInfo format ["DEDS w_018 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.75 }
triScreenshot "w_019"
logInfo format ["DEDS w_019 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.00 }
triScreenshot "w_020"
logInfo format ["DEDS w_020 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.25 }
triScreenshot "w_021"
logInfo format ["DEDS w_021 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.50 }
triScreenshot "w_022"
logInfo format ["DEDS w_022 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.75 }
triScreenshot "w_023"
logInfo format ["DEDS w_023 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.00 }
triScreenshot "w_024"
logInfo format ["DEDS w_024 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.25 }
triScreenshot "w_025"
logInfo format ["DEDS w_025 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.50 }
triScreenshot "w_026"
logInfo format ["DEDS w_026 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.75 }
triScreenshot "w_027"
logInfo format ["DEDS w_027 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.00 }
triScreenshot "w_028"
logInfo format ["DEDS w_028 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.25 }
triScreenshot "w_029"
logInfo format ["DEDS w_029 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.50 }
triScreenshot "w_030"
logInfo format ["DEDS w_030 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.75 }
triScreenshot "w_031"
logInfo format ["DEDS w_031 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.00 }
triScreenshot "w_032"
logInfo format ["DEDS w_032 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.25 }
triScreenshot "w_033"
logInfo format ["DEDS w_033 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.50 }
triScreenshot "w_034"
logInfo format ["DEDS w_034 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.75 }
triScreenshot "w_035"
logInfo format ["DEDS w_035 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.00 }
triScreenshot "w_036"
logInfo format ["DEDS w_036 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.25 }
triScreenshot "w_037"
logInfo format ["DEDS w_037 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.50 }
triScreenshot "w_038"
logInfo format ["DEDS w_038 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.75 }
triScreenshot "w_039"
logInfo format ["DEDS w_039 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.00 }
triScreenshot "w_040"
logInfo format ["DEDS w_040 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.25 }
triScreenshot "w_041"
logInfo format ["DEDS w_041 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.50 }
triScreenshot "w_042"
logInfo format ["DEDS w_042 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.75 }
triScreenshot "w_043"
logInfo format ["DEDS w_043 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.00 }
triScreenshot "w_044"
logInfo format ["DEDS w_044 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.25 }
triScreenshot "w_045"
logInfo format ["DEDS w_045 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.50 }
triScreenshot "w_046"
logInfo format ["DEDS w_046 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.75 }
triScreenshot "w_047"
logInfo format ["DEDS w_047 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.00 }
triScreenshot "w_048"
logInfo format ["DEDS w_048 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.25 }
triScreenshot "w_049"
logInfo format ["DEDS w_049 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.50 }
triScreenshot "w_050"
logInfo format ["DEDS w_050 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.75 }
triScreenshot "w_051"
logInfo format ["DEDS w_051 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.00 }
triScreenshot "w_052"
logInfo format ["DEDS w_052 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.25 }
triScreenshot "w_053"
logInfo format ["DEDS w_053 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.50 }
triScreenshot "w_054"
logInfo format ["DEDS w_054 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.75 }
triScreenshot "w_055"
logInfo format ["DEDS w_055 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.00 }
triScreenshot "w_056"
logInfo format ["DEDS w_056 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.25 }
triScreenshot "w_057"
logInfo format ["DEDS w_057 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.50 }
triScreenshot "w_058"
logInfo format ["DEDS w_058 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.75 }
triScreenshot "w_059"
logInfo format ["DEDS w_059 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.00 }
triScreenshot "w_060"
logInfo format ["DEDS w_060 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.25 }
triScreenshot "w_061"
logInfo format ["DEDS w_061 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.50 }
triScreenshot "w_062"
logInfo format ["DEDS w_062 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.75 }
triScreenshot "w_063"
logInfo format ["DEDS w_063 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 16.00 }
triScreenshot "w_064"
logInfo format ["DEDS w_064 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 16.25 }
triScreenshot "w_065"
logInfo format ["DEDS w_065 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 16.50 }
triScreenshot "w_066"
logInfo format ["DEDS w_066 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 16.75 }
triScreenshot "w_067"
logInfo format ["DEDS w_067 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 17.00 }
triScreenshot "w_068"
logInfo format ["DEDS w_068 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 17.25 }
triScreenshot "w_069"
logInfo format ["DEDS w_069 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 17.50 }
triScreenshot "w_070"
logInfo format ["DEDS w_070 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 17.75 }
triScreenshot "w_071"
logInfo format ["DEDS w_071 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 18.00 }
triScreenshot "w_072"
logInfo format ["DEDS w_072 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 18.25 }
triScreenshot "w_073"
logInfo format ["DEDS w_073 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 18.50 }
triScreenshot "w_074"
logInfo format ["DEDS w_074 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 18.75 }
triScreenshot "w_075"
logInfo format ["DEDS w_075 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 19.00 }
triScreenshot "w_076"
logInfo format ["DEDS w_076 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 19.25 }
triScreenshot "w_077"
logInfo format ["DEDS w_077 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 19.50 }
triScreenshot "w_078"
logInfo format ["DEDS w_078 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 19.75 }
triScreenshot "w_079"
logInfo format ["DEDS w_079 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 20.00 }
triScreenshot "w_080"
logInfo format ["DEDS w_080 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 20.25 }
triScreenshot "w_081"
logInfo format ["DEDS w_081 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 20.50 }
triScreenshot "w_082"
logInfo format ["DEDS w_082 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 20.75 }
triScreenshot "w_083"
logInfo format ["DEDS w_083 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 21.00 }
triScreenshot "w_084"
logInfo format ["DEDS w_084 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 21.25 }
triScreenshot "w_085"
logInfo format ["DEDS w_085 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 21.50 }
triScreenshot "w_086"
logInfo format ["DEDS w_086 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 21.75 }
triScreenshot "w_087"
logInfo format ["DEDS w_087 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 22.00 }
triScreenshot "w_088"
logInfo format ["DEDS w_088 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 22.25 }
triScreenshot "w_089"
logInfo format ["DEDS w_089 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 22.50 }
triScreenshot "w_090"
logInfo format ["DEDS w_090 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 22.75 }
triScreenshot "w_091"
logInfo format ["DEDS w_091 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 23.00 }
triScreenshot "w_092"
logInfo format ["DEDS w_092 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 23.25 }
triScreenshot "w_093"
logInfo format ["DEDS w_093 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 23.50 }
triScreenshot "w_094"
logInfo format ["DEDS w_094 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 23.75 }
triScreenshot "w_095"
logInfo format ["DEDS w_095 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 24.00 }
if (true) then { doStop ded; ded doMove (getPos ded) }
logInfo format ["DEDA walk stop %1 %2 moved %3", time, getPosASL ded, (getPosASL ded) distance daP0]
triScreenshot "w_096"
logInfo format ["DEDS w_096 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 24.25 }
triScreenshot "w_097"
logInfo format ["DEDS w_097 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 24.50 }
triScreenshot "w_098"
logInfo format ["DEDS w_098 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 24.75 }
triScreenshot "w_099"
logInfo format ["DEDS w_099 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 25.00 }
triScreenshot "w_100"
logInfo format ["DEDS w_100 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 25.25 }
triScreenshot "w_101"
logInfo format ["DEDS w_101 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 25.50 }
triScreenshot "w_102"
logInfo format ["DEDS w_102 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 25.75 }
triScreenshot "w_103"
logInfo format ["DEDS w_103 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 26.00 }
triScreenshot "w_104"
logInfo format ["DEDS w_104 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 26.25 }
triScreenshot "w_105"
logInfo format ["DEDS w_105 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 26.50 }
triScreenshot "w_106"
logInfo format ["DEDS w_106 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 26.75 }
triScreenshot "w_107"
logInfo format ["DEDS w_107 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 27.00 }
triScreenshot "w_108"
logInfo format ["DEDS w_108 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 27.25 }
triScreenshot "w_109"
logInfo format ["DEDS w_109 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 27.50 }
triScreenshot "w_110"
logInfo format ["DEDS w_110 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 27.75 }
triScreenshot "w_111"
logInfo format ["DEDS w_111 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 28.00 }
triScreenshot "w_112"
logInfo format ["DEDS w_112 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 28.25 }
triScreenshot "w_113"
logInfo format ["DEDS w_113 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 28.50 }
triScreenshot "w_114"
logInfo format ["DEDS w_114 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 28.75 }
triScreenshot "w_115"
logInfo format ["DEDS w_115 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 29.00 }
triScreenshot "w_116"
logInfo format ["DEDS w_116 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 29.25 }
triScreenshot "w_117"
logInfo format ["DEDS w_117 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 29.50 }
triScreenshot "w_118"
logInfo format ["DEDS w_118 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 29.75 }
triScreenshot "w_119"
logInfo format ["DEDS w_119 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 30.00 }
triScreenshot "w_120"
logInfo format ["DEDS w_120 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 30.25 }
triScreenshot "w_121"
logInfo format ["DEDS w_121 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 30.50 }
triScreenshot "w_122"
logInfo format ["DEDS w_122 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 30.75 }
triScreenshot "w_123"
logInfo format ["DEDS w_123 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
logInfo format ["DEDA walk end %1 %2 moved %3 move %4", time, getPosASL ded, (getPosASL ded) distance daP0, getMove ded]
triAssert [((getPosASL ded) distance daP0) > 33.2]
triAssertEq [getMove ded, "dedidle"]
// ---- the turn: told to watch a point behind him, he must come round on the spot
if (true) then { daT0 = time; daD0 = getDir ded; daTurnBy = "doWatch"; ded doWatch [4400, 11200, 0] }
logInfo format ["DEDA turn start %1 %2 dir %3", time, getPosASL ded, getDir ded]
triSimUntil { time >= daT0 + 0.00 }
triScreenshot "t_000"
logInfo format ["DEDS t_000 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.25 }
triScreenshot "t_001"
logInfo format ["DEDS t_001 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.50 }
triScreenshot "t_002"
logInfo format ["DEDS t_002 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.75 }
triScreenshot "t_003"
logInfo format ["DEDS t_003 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.00 }
triScreenshot "t_004"
logInfo format ["DEDS t_004 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.25 }
triScreenshot "t_005"
logInfo format ["DEDS t_005 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.50 }
triScreenshot "t_006"
logInfo format ["DEDS t_006 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.75 }
triScreenshot "t_007"
logInfo format ["DEDS t_007 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.00 }
triScreenshot "t_008"
logInfo format ["DEDS t_008 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.25 }
triScreenshot "t_009"
logInfo format ["DEDS t_009 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.50 }
triScreenshot "t_010"
logInfo format ["DEDS t_010 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.75 }
triScreenshot "t_011"
logInfo format ["DEDS t_011 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.00 }
if ((cos ((getDir ded) - daD0)) > 0.97) then { daTurnBy = "doMove"; ded doMove [4500, 11200, 0] }
logInfo format ["DEDA turn by %1 %2 dir %3", daTurnBy, time, getDir ded]
triScreenshot "t_012"
logInfo format ["DEDS t_012 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.25 }
triScreenshot "t_013"
logInfo format ["DEDS t_013 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.50 }
triScreenshot "t_014"
logInfo format ["DEDS t_014 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.75 }
triScreenshot "t_015"
logInfo format ["DEDS t_015 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.00 }
triScreenshot "t_016"
logInfo format ["DEDS t_016 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.25 }
triScreenshot "t_017"
logInfo format ["DEDS t_017 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.50 }
triScreenshot "t_018"
logInfo format ["DEDS t_018 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.75 }
triScreenshot "t_019"
logInfo format ["DEDS t_019 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.00 }
triScreenshot "t_020"
logInfo format ["DEDS t_020 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.25 }
triScreenshot "t_021"
logInfo format ["DEDS t_021 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.50 }
triScreenshot "t_022"
logInfo format ["DEDS t_022 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.75 }
triScreenshot "t_023"
logInfo format ["DEDS t_023 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.00 }
triScreenshot "t_024"
logInfo format ["DEDS t_024 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.25 }
triScreenshot "t_025"
logInfo format ["DEDS t_025 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.50 }
triScreenshot "t_026"
logInfo format ["DEDS t_026 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.75 }
triScreenshot "t_027"
logInfo format ["DEDS t_027 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.00 }
triScreenshot "t_028"
logInfo format ["DEDS t_028 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.25 }
triScreenshot "t_029"
logInfo format ["DEDS t_029 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.50 }
triScreenshot "t_030"
logInfo format ["DEDS t_030 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.75 }
triScreenshot "t_031"
logInfo format ["DEDS t_031 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.00 }
triScreenshot "t_032"
logInfo format ["DEDS t_032 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.25 }
triScreenshot "t_033"
logInfo format ["DEDS t_033 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.50 }
triScreenshot "t_034"
logInfo format ["DEDS t_034 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.75 }
triScreenshot "t_035"
logInfo format ["DEDS t_035 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.00 }
triScreenshot "t_036"
logInfo format ["DEDS t_036 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.25 }
triScreenshot "t_037"
logInfo format ["DEDS t_037 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.50 }
triScreenshot "t_038"
logInfo format ["DEDS t_038 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.75 }
triScreenshot "t_039"
logInfo format ["DEDS t_039 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.00 }
triScreenshot "t_040"
logInfo format ["DEDS t_040 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.25 }
triScreenshot "t_041"
logInfo format ["DEDS t_041 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.50 }
triScreenshot "t_042"
logInfo format ["DEDS t_042 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.75 }
triScreenshot "t_043"
logInfo format ["DEDS t_043 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.00 }
triScreenshot "t_044"
logInfo format ["DEDS t_044 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.25 }
triScreenshot "t_045"
logInfo format ["DEDS t_045 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.50 }
triScreenshot "t_046"
logInfo format ["DEDS t_046 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.75 }
triScreenshot "t_047"
logInfo format ["DEDS t_047 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.00 }
triScreenshot "t_048"
logInfo format ["DEDS t_048 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.25 }
triScreenshot "t_049"
logInfo format ["DEDS t_049 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.50 }
triScreenshot "t_050"
logInfo format ["DEDS t_050 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.75 }
triScreenshot "t_051"
logInfo format ["DEDS t_051 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.00 }
triScreenshot "t_052"
logInfo format ["DEDS t_052 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.25 }
triScreenshot "t_053"
logInfo format ["DEDS t_053 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.50 }
triScreenshot "t_054"
logInfo format ["DEDS t_054 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.75 }
triScreenshot "t_055"
logInfo format ["DEDS t_055 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.00 }
triScreenshot "t_056"
logInfo format ["DEDS t_056 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.25 }
triScreenshot "t_057"
logInfo format ["DEDS t_057 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.50 }
triScreenshot "t_058"
logInfo format ["DEDS t_058 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.75 }
triScreenshot "t_059"
logInfo format ["DEDS t_059 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.00 }
triScreenshot "t_060"
logInfo format ["DEDS t_060 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.25 }
triScreenshot "t_061"
logInfo format ["DEDS t_061 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.50 }
triScreenshot "t_062"
logInfo format ["DEDS t_062 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.75 }
triScreenshot "t_063"
logInfo format ["DEDS t_063 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 16.00 }
triScreenshot "t_064"
logInfo format ["DEDS t_064 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 16.25 }
triScreenshot "t_065"
logInfo format ["DEDS t_065 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 16.50 }
triScreenshot "t_066"
logInfo format ["DEDS t_066 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 16.75 }
triScreenshot "t_067"
logInfo format ["DEDS t_067 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 17.00 }
triScreenshot "t_068"
logInfo format ["DEDS t_068 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 17.25 }
triScreenshot "t_069"
logInfo format ["DEDS t_069 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 17.50 }
triScreenshot "t_070"
logInfo format ["DEDS t_070 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 17.75 }
triScreenshot "t_071"
logInfo format ["DEDS t_071 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 18.00 }
triScreenshot "t_072"
logInfo format ["DEDS t_072 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 18.25 }
triScreenshot "t_073"
logInfo format ["DEDS t_073 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 18.50 }
triScreenshot "t_074"
logInfo format ["DEDS t_074 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 18.75 }
triScreenshot "t_075"
logInfo format ["DEDS t_075 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 19.00 }
triScreenshot "t_076"
logInfo format ["DEDS t_076 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 19.25 }
triScreenshot "t_077"
logInfo format ["DEDS t_077 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 19.50 }
triScreenshot "t_078"
logInfo format ["DEDS t_078 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 19.75 }
triScreenshot "t_079"
logInfo format ["DEDS t_079 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
logInfo format ["DEDA turn end %1 %2 dir %3 move %4 footfalls %5", time, getPosASL ded, getDir ded, getMove ded, KJ_footfallCount]
triAssert [(cos ((getDir ded) - daD0)) < -0.9]
// ---- a short walk back west: the goal is near, so the AI holds him in the slow band
if (true) then { daT0 = time; daP0 = getPosASL ded; ded doWatch objNull; ded doMove [((getPosASL ded) select 0) - 45, 11200, 0] }
logInfo format ["DEDA back start %1 %2", time, getPosASL ded]
triSimUntil { time >= daT0 + 0.00 }
triScreenshot "b_000"
logInfo format ["DEDS b_000 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.25 }
triScreenshot "b_001"
logInfo format ["DEDS b_001 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.50 }
triScreenshot "b_002"
logInfo format ["DEDS b_002 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 0.75 }
triScreenshot "b_003"
logInfo format ["DEDS b_003 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.00 }
triScreenshot "b_004"
logInfo format ["DEDS b_004 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.25 }
triScreenshot "b_005"
logInfo format ["DEDS b_005 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.50 }
triScreenshot "b_006"
logInfo format ["DEDS b_006 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 1.75 }
triScreenshot "b_007"
logInfo format ["DEDS b_007 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.00 }
triScreenshot "b_008"
logInfo format ["DEDS b_008 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.25 }
triScreenshot "b_009"
logInfo format ["DEDS b_009 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.50 }
triScreenshot "b_010"
logInfo format ["DEDS b_010 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 2.75 }
triScreenshot "b_011"
logInfo format ["DEDS b_011 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.00 }
triScreenshot "b_012"
logInfo format ["DEDS b_012 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.25 }
triScreenshot "b_013"
logInfo format ["DEDS b_013 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.50 }
triScreenshot "b_014"
logInfo format ["DEDS b_014 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 3.75 }
triScreenshot "b_015"
logInfo format ["DEDS b_015 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.00 }
triScreenshot "b_016"
logInfo format ["DEDS b_016 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.25 }
triScreenshot "b_017"
logInfo format ["DEDS b_017 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.50 }
triScreenshot "b_018"
logInfo format ["DEDS b_018 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 4.75 }
triScreenshot "b_019"
logInfo format ["DEDS b_019 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.00 }
triScreenshot "b_020"
logInfo format ["DEDS b_020 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.25 }
triScreenshot "b_021"
logInfo format ["DEDS b_021 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.50 }
triScreenshot "b_022"
logInfo format ["DEDS b_022 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 5.75 }
triScreenshot "b_023"
logInfo format ["DEDS b_023 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.00 }
triScreenshot "b_024"
logInfo format ["DEDS b_024 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.25 }
triScreenshot "b_025"
logInfo format ["DEDS b_025 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.50 }
triScreenshot "b_026"
logInfo format ["DEDS b_026 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 6.75 }
triScreenshot "b_027"
logInfo format ["DEDS b_027 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.00 }
triScreenshot "b_028"
logInfo format ["DEDS b_028 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.25 }
triScreenshot "b_029"
logInfo format ["DEDS b_029 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.50 }
triScreenshot "b_030"
logInfo format ["DEDS b_030 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 7.75 }
triScreenshot "b_031"
logInfo format ["DEDS b_031 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.00 }
triScreenshot "b_032"
logInfo format ["DEDS b_032 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.25 }
triScreenshot "b_033"
logInfo format ["DEDS b_033 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.50 }
triScreenshot "b_034"
logInfo format ["DEDS b_034 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 8.75 }
triScreenshot "b_035"
logInfo format ["DEDS b_035 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.00 }
triScreenshot "b_036"
logInfo format ["DEDS b_036 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.25 }
triScreenshot "b_037"
logInfo format ["DEDS b_037 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.50 }
triScreenshot "b_038"
logInfo format ["DEDS b_038 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 9.75 }
triScreenshot "b_039"
logInfo format ["DEDS b_039 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.00 }
triScreenshot "b_040"
logInfo format ["DEDS b_040 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.25 }
triScreenshot "b_041"
logInfo format ["DEDS b_041 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.50 }
triScreenshot "b_042"
logInfo format ["DEDS b_042 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 10.75 }
triScreenshot "b_043"
logInfo format ["DEDS b_043 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.00 }
triScreenshot "b_044"
logInfo format ["DEDS b_044 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.25 }
triScreenshot "b_045"
logInfo format ["DEDS b_045 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.50 }
triScreenshot "b_046"
logInfo format ["DEDS b_046 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 11.75 }
triScreenshot "b_047"
logInfo format ["DEDS b_047 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.00 }
triScreenshot "b_048"
logInfo format ["DEDS b_048 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.25 }
triScreenshot "b_049"
logInfo format ["DEDS b_049 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.50 }
triScreenshot "b_050"
logInfo format ["DEDS b_050 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 12.75 }
triScreenshot "b_051"
logInfo format ["DEDS b_051 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.00 }
triScreenshot "b_052"
logInfo format ["DEDS b_052 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.25 }
triScreenshot "b_053"
logInfo format ["DEDS b_053 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.50 }
triScreenshot "b_054"
logInfo format ["DEDS b_054 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 13.75 }
triScreenshot "b_055"
logInfo format ["DEDS b_055 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.00 }
triScreenshot "b_056"
logInfo format ["DEDS b_056 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.25 }
triScreenshot "b_057"
logInfo format ["DEDS b_057 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.50 }
triScreenshot "b_058"
logInfo format ["DEDS b_058 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 14.75 }
triScreenshot "b_059"
logInfo format ["DEDS b_059 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.00 }
triScreenshot "b_060"
logInfo format ["DEDS b_060 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.25 }
triScreenshot "b_061"
logInfo format ["DEDS b_061 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.50 }
triScreenshot "b_062"
logInfo format ["DEDS b_062 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
triSimUntil { time >= daT0 + 15.75 }
triScreenshot "b_063"
logInfo format ["DEDS b_063 %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }
logInfo format ["DEDA back end %1 %2 moved %3 move %4 footfalls %5", time, getPosASL ded, (getPosASL ded) distance daP0, getMove ded, KJ_footfallCount]
if (true) then { DEDW_QUIT = true }
triAssert [alive ded]
triAssert [((getPosASL ded) distance daP0) > 8]
triAssert [KJ_footfallCount >= 8]
triEndTest
