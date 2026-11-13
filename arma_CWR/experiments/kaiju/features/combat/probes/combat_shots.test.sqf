// ============================================================================
//  Dedushka COMBAT, a photo run (a diagnostic, not part of the gate). Nothing
//  fires: hits and wounds are given by script, so every frame can be timed.
//    day    what a hit looks like where it lands (furnace, flank, leg), then
//           each wound stage from the front;
//    night  each wound stage again (the gutter light), then the whole death.
//  Frames land in the run folder; the best are copied to the notes' shots\.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
setDate [1985, 6, 10, 15, 30]
0 setOvercast 0.3
0 setFog 0
setViewDistance 2000
showCinemaBorder false
player setPos [4690, 11120, 0]
call compile preprocessFile "kjt.sqf"
call compile preprocessFile "kjt_class.sqf"
{ deleteVehicle _x } forEach [kjcF1, kjcF2, kjcF3]
[kjtClass, 0] call kjtSpawn
triSimFrames 5
triAssert [!(isNull kjtDed)]
call kjtHold
[kjtDed] exec "kj\combat\start.sqs"
triSimUntil { !(isNil "KJCB_ready") }
triSimFrames 30
kjoCam = "camera" camCreate [4900, 11000, 30]
kjoCam cameraEffect ["internal", "back"]
if (true) then { kjoCam camSetTarget kjtDed; kjoCam camSetFov 0.7; kjoCam camCommit 0 }
// a T-80 and three men for scale, 45 m in front of him, off his line
kjoC = createCenter civilian
kjoG = createGroup civilian
kjoTank = "T80" createVehicle [4690, 11300, 0]
if (true) then { kjoTank setPos ([48, 22] call kjtRel); kjoTank setDir 200 }
{ "Civilian" createUnit [[4690, 11310, 0], kjoG, "this setPos ([46, 14] call kjtRel); this stop true; this disableAI ""MOVE""", 0.5, "PRIVATE"] } forEach [1]
triSimFrames 40
triSetView ([25, 105, 3, 24] call kjtView)
triSimFrames 10
triScreenshot "o00_discard"
triScreenshot "o01_unhurt_day"
// ---- what a hit looks like -------------------------------------------------------------
kjoTo = { [kjtDed, (KJ_woundHealth - _this) * KJCB_POOL, objNull] call KJCB_fnWound }
kjoHit = { private ["_i"]; _i = 0; while { _i < (_this select 1) } do { [kjtDed, _this select 0, _this select 2, objNull] call KJCB_fnScriptHit; _i = _i + 1 } }
["Heat120", 1, KJCB_furnace] call kjoHit
triSimFrames 2
triScreenshot "o02_hit_furnace"
triSimFrames 12
triScreenshot "o03_hit_furnace_after"
triSimFrames 150
["Heat120", 1, [0.2 * KJCB_H, 0.1 * KJCB_H, 0.62 * KJCB_H]] call kjoHit
triSimFrames 2
triScreenshot "o04_hit_arm"
triSimFrames 150
["Bullet30E", 12, KJCB_furnace] call kjoHit
triSimFrames 3
triScreenshot "o05_stings"
triSimFrames 150
// ---- the stages, by day ----------------------------------------------------------------
0.80 call kjoTo
triSimFrames 6
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 9 }
triScreenshot "o06_stage1_stung_day"
0.55 call kjoTo
triSimFrames 6
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 3 }
triScreenshot "o07_stage2_roar_of_pain"
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 11 }
triScreenshot "o08_stage2_wounded_day"
0.30 call kjoTo
triSimFrames 6
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 10 }
triScreenshot "o09_stage3_badly_hurt_day"
0.10 call kjoTo
triSimFrames 6
triSimUntil { time > ((KJ_woundLog select ((count KJ_woundLog) - 1)) select 0) + 10 }
triScreenshot "o10_stage4_dying_day"
triSetView ([8, 55, 2, 27] call kjtView)
triSimFrames 40
triScreenshot "o11_stage4_from_his_feet"
// ---- night -------------------------------------------------------------------------------
setDate [1985, 6, 10, 22, 40]
triSetView ([25, 105, 3, 24] call kjtView)
triSimFrames 120
triScreenshot "o12_stage4_night"
triSimFrames 30
triScreenshot "o13_stage4_night_b"
triSimFrames 45
triScreenshot "o14_stage4_night_c"
// ---- the death, at night ------------------------------------------------------------------
if (true) then { kjoT0 = time; [kjtDed, 10000, objNull] call KJCB_fnWound }
triSimUntil { KJ_deathPhase == "stagger" }
triSimUntil { time > kjoT0 + 2.5 }
triScreenshot "o15_death_stagger"
triSimUntil { KJ_deathPhase == "flare" }
triSimUntil { (KJ_deathPhase != "flare") or (time > ((KJ_deathLog select 1) select 1) + 0.5 * KJCB_D_FLARE) }
triScreenshot "o16_death_flare_rising"
triSimUntil { (KJ_deathPhase != "flare") or (time > ((KJ_deathLog select 1) select 1) + 0.95 * KJCB_D_FLARE) }
triScreenshot "o17_death_flare_white"
triSimUntil { KJ_deathPhase == "vent" }
triSimFrames 2
triScreenshot "o18_death_vent"
triSimFrames 6
triScreenshot "o19_death_vent_ring"
triSimUntil { KJ_deathPhase == "dark" }
triSimUntil { time > ((KJ_deathLog select 3) select 1) + 1.5 }
triScreenshot "o20_death_dark"
triSimUntil { KJ_deathPhase == "fall" }
triSimUntil { time > ((KJ_deathLog select 4) select 1) + 0.5 * KJCB_D_FALL }
triScreenshot "o21_death_falling"
triSimUntil { KJ_deathPhase == "down" }
triSimFrames 4
triScreenshot "o22_death_down"
triSimUntil { time > ((KJ_deathLog select 5) select 1) + 2 }
triScreenshot "o23_death_dust"
triSimUntil { KJ_deathPhase == "body" }
triSimFrames 200
triSetView ([70, 95, 5, 5, 20] call kjtView)
triSimFrames 10
triScreenshot "o24_body_night"
setDate [1985, 6, 11, 6, 10]
triSimFrames 400
triScreenshot "o25_body_dawn"
triSetView ([105, 42, 1.7, 4, 12] call kjtView)
triSimFrames 10
triScreenshot "o26_body_dawn_close"
logInfo format ["KJO done: death log %1, stats %2", KJ_deathLog, KJ_deathStats]
triAssert [!(alive kjtDed)]
triEndTest
