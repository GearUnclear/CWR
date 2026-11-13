// ============================================================================
//  Dedushka clips: every move played on the creature and photographed at its key moments
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
// ground heights (ASL) on a 25 m grid, 350 m by 500 m
daGz = 10750
while { daGz <= 11250 } do { daRow = []; daGx = 4625; while { daGx <= 4975 } do { daProbe setPos [daGx, daGz, 0]; daRow = daRow + [(getPosASL daProbe) select 2]; daGx = daGx + 25 }; logInfo format ["DEDG %1 %2 %3", daGz, 4625, daRow]; daGz = daGz + 25 }
triSimFrames 200
logInfo format ["DEDA settled %1 %2 %3 %4 alive %5", time, getMove ded, getPosASL ded, getDir ded, alive ded]
triAssertEq [getMove ded, "dedidle"]
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 6
triScreenshot "00_warmup_discard"
// ---- DedIdle (he spawns into it)
if (true) then { daT0 = time }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "idle_1_front"
logInfo format ["DEDS idle_1_front %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 3.0 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "idle_2_side"
logInfo format ["DEDS idle_2_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 6.0 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "idle_3_front"
logInfo format ["DEDS idle_3_front %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
// ---- DedWalk (7.0 s, loop)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedWalk" }
logInfo format ["DEDA play DedWalk %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedwalk"]
triSimUntil { time > daT0 + 1.40 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "walk_1_right_foot_coming_down"
logInfo format ["DEDS walk_1_right_foot_coming_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 2.40 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "walk_2_right_foot_down"
logInfo format ["DEDS walk_2_right_foot_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 6.42 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "walk_3_mid_stride"
logInfo format ["DEDS walk_3_mid_stride %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 8.23 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "walk_4_left_foot_down"
logInfo format ["DEDS walk_4_left_foot_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 13.87 }
logInfo format ["DEDA walkcycle moved %1 m in one played cycle, step 16.150, move %2", (getPosASL ded) distance daP0, getMove ded]
triAssertEq [getMove ded, "dedidle"]
triAssert [abs (((getPosASL ded) distance daP0) - 16.150) < 2.5]
// ---- DedTurnL (6.0 s, loop)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedTurnL" }
logInfo format ["DEDA play DedTurnL %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedturnl"]
triSimUntil { time > daT0 + 0.48 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "turnl_1_foot_up"
logInfo format ["DEDS turnl_1_foot_up %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 1.38 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "turnl_2_foot_down"
logInfo format ["DEDS turnl_2_foot_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 4.38 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "turnl_3_other_foot_down"
logInfo format ["DEDS turnl_3_other_foot_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 8.00 }
triAssertEq [getMove ded, "dedidle"]
// ---- DedTurnR (6.0 s, loop)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedTurnR" }
logInfo format ["DEDA play DedTurnR %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedturnr"]
triSimUntil { time > daT0 + 0.48 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "turnr_1_foot_up"
logInfo format ["DEDS turnr_1_foot_up %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 1.38 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "turnr_2_foot_down"
logInfo format ["DEDS turnr_2_foot_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 4.38 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "turnr_3_other_foot_down"
logInfo format ["DEDS turnr_3_other_foot_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 8.00 }
triAssertEq [getMove ded, "dedidle"]
// ---- DedRoar (9.0 s)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedRoar" }
logInfo format ["DEDA play DedRoar %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedroar"]
triSimUntil { time > daT0 + 2.40 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "roar_1_head_down"
logInfo format ["DEDS roar_1_head_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 3.70 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "roar_2_thrown_up"
logInfo format ["DEDS roar_2_thrown_up %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 5.00 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "roar_3_full_cry"
logInfo format ["DEDS roar_3_full_cry %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 5.90 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "roar_4_full_cry_side"
logInfo format ["DEDS roar_4_full_cry_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 8.00 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "roar_5_spent"
logInfo format ["DEDS roar_5_spent %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 10.80 }
logInfo format ["DEDA after DedRoar %1 %2", time, getMove ded]
triAssertEq [getMove ded, "dedidle"]
// ---- DedBeamRise (6.5 s)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedBeamRise" }
logInfo format ["DEDA play DedBeamRise %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedbeamrise"]
triSimUntil { time > daT0 + 0.30 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "beamrise_1_start_side"
logInfo format ["DEDS beamrise_1_start_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 2.93 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "beamrise_2_rising_side"
logInfo format ["DEDS beamrise_2_rising_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 6.30 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "beamrise_3_tall_side"
logInfo format ["DEDS beamrise_3_tall_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 8.00 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "beamhold_1_front"
logInfo format ["DEDS beamhold_1_front %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triAssertEq [getMove ded, "dedbeamhold"]
triSimUntil { time > daT0 + 15.68 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "beamhold_2_side"
logInfo format ["DEDS beamhold_2_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
logInfo format ["DEDA hold kept %1 %2", time, getMove ded]
triAssertEq [getMove ded, "dedbeamhold"]
// ---- DedBeamLower (7.5 s)
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedBeamLower" }
logInfo format ["DEDA play DedBeamLower %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedbeamlower"]
triSimUntil { time > daT0 + 2.20 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "beamlower_1_sagging_side"
logInfo format ["DEDS beamlower_1_sagging_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 4.50 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "beamlower_2_lowest_side"
logInfo format ["DEDS beamlower_2_lowest_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 9.30 }
triAssertEq [getMove ded, "dedidle"]
// ---- DedStomp (8.0 s)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedStomp" }
logInfo format ["DEDA play DedStomp %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedstomp"]
triSimUntil { time > daT0 + 4.20 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "stomp_1_foot_up"
logInfo format ["DEDS stomp_1_foot_up %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 5.20 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "stomp_2_falling"
logInfo format ["DEDS stomp_2_falling %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 5.62 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "stomp_3_impact"
logInfo format ["DEDS stomp_3_impact %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 6.10 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "stomp_4_after_side"
logInfo format ["DEDS stomp_4_after_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 9.80 }
logInfo format ["DEDA after DedStomp %1 %2", time, getMove ded]
triAssertEq [getMove ded, "dedidle"]
// ---- DedSwat (9.0 s)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedSwat" }
logInfo format ["DEDA play DedSwat %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedswat"]
triSimUntil { time > daT0 + 2.70 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "swat_1_arm_raised"
logInfo format ["DEDS swat_1_arm_raised %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 4.03 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "swat_2_sweep_early"
logInfo format ["DEDS swat_2_sweep_early %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 4.78 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "swat_3_sweep_mid"
logInfo format ["DEDS swat_3_sweep_mid %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 5.53 }
triSetView ([215, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "swat_4_sweep_late"
logInfo format ["DEDS swat_4_sweep_late %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 10.80 }
logInfo format ["DEDA after DedSwat %1 %2", time, getMove ded]
triAssertEq [getMove ded, "dedidle"]
// ---- DedFlinch (2.6 s)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedFlinch" }
logInfo format ["DEDA play DedFlinch %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "dedflinch"]
triSimUntil { time > daT0 + 0.40 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "flinch_1_deepest"
logInfo format ["DEDS flinch_1_deepest %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 0.90 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "flinch_2_ringing"
logInfo format ["DEDS flinch_2_ringing %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 4.40 }
logInfo format ["DEDA after DedFlinch %1 %2", time, getMove ded]
triAssertEq [getMove ded, "dedidle"]
// ---- DedDormant (16.0 s, loop)
call daHold
triSimFrames 40
if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "DedDormant" }
logInfo format ["DEDA play DedDormant %1 from %2", time, getMove ded]
triSimUntil { time > daT0 + 0.5 }
triAssertEq [getMove ded, "deddormant"]
triSimUntil { time > daT0 + 5.00 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "dormant_1_sunk_front"
logInfo format ["DEDS dormant_1_sunk_front %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 8.00 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "dormant_2_sunk_side"
logInfo format ["DEDS dormant_2_sunk_side %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 20.00 }
logInfo format ["DEDA dormant kept %1 %2", time, getMove ded]
triAssertEq [getMove ded, "deddormant"]
if (true) then { daT0 = time; ded playMove "DedIdle" }
triSimUntil { time > daT0 + 5.0 }
triSetView ([150, 84.0, 8.0, 18.799999999999997] call daView)
triSimFrames 2
triScreenshot "dormant_3_woken"
logInfo format ["DEDS dormant_3_woken %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triAssertEq [getMove ded, "dedidle"]
// ---- death
triAssert [alive ded]
if (true) then { daT0 = time; ded allowDammage true; ded setDammage 1 }
triSimUntil { time > daT0 + 2.75 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "die_1_falling"
logInfo format ["DEDS die_1_falling %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triSimUntil { time > daT0 + 7.00 }
triSetView ([95, 88.0, 2.4, 18.400000000000002] call daView)
triSimFrames 2
triScreenshot "die_2_down"
logInfo format ["DEDS die_2_down %1 %2 %3 %4", time, getMove ded, getPosASL ded, getDir ded]
triAssert [!(alive ded)]
logInfo format ["DEDA done %1 animChanges %2", time, daAnim]
triEndTest
