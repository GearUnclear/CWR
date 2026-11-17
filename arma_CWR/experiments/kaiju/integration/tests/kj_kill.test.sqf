// ============================================================================
//  Dedushka integration: the ending in which a heavy force kills him, STAGED
//  (no weapon fired). The demo mission on the take "short,kill,staged" (he
//  starts nearer the town; the plan ends glow, wake, through, roar, fire, beam,
//  spent, kill; KJD_killLive is false). Needs the combat feature in the script
//  set (kj\combat); without it the test says so and fails. The real fight,
//  with weapons, is kj_fight.
//
//      run-kj.ps1 -Name kj_kill -Params short,kill,staged
//
//  The fire of the kill beat is staged (kjd_fire.sqs: no weapon is fired; each
//  shot is a direct hit by a tank's HEAT round through the combat feature's own
//  entry point). His health, his wounds and his death are the combat feature's.
//  What is checked: the beats before it still play with the combat feature in
//  the set; he is wounded by stages, dies, everything of the set stops, and
//  the body stays. Soft checks; the verdict is the KJK RESULT line.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
triSimUntil { !(isNil "KJ_ready") }
triSimUntil { KJ_ready }
kjFails = []
kjChk = { if (!(_this select 0)) then { kjFails = kjFails + [_this select 1]; logInfo format ["KJK CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJK ok: %1", _this select 1] } }
if (true) then { KJD_spentHold = 10; KJB_SPENT = 30; KJ_beamOverride = { KJ_beamT_spent = 30 }; KJT_on = true; [kaiju] exec "kj\brain\watch.sqs"; call compile preprocessFile "kjd_cams.sqf"; KJD_probe = "HeliHEmpty" createVehicle [5100, 4100, 0] }
kjCamObj = "camera" camCreate [5000, 3980, 3]
kjCamObj cameraEffect ["internal", "back"]
showCinemaBorder false
enableRadio false
kjView = { kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov (_this call KJD_fnLens); kjCamObj camCommit 0; [_this, kaiju] call KJD_fnView }
kjT0 = time
logInfo format ["KJK start: creature %1, native %2, plan %3, combat feature %4, unkillable %5, health %6", typeOf kaiju, KJ_native, KJD_plan, KJ_hasCombat, KJ_unkillable, if (isNil "KJ_woundHealth") then { -1 } else { KJ_woundHealth }]
// without the combat feature its globals do not exist: give them a value so the lines below can still be logged
if (isNil "KJ_woundHealth") then { KJ_woundHealth = -1; KJ_deathPhase = ""; KJ_deathLog = []; KJ_deathStats = []; KJ_combatStats = []; KJ_combatByAmmo = [] }
[KJ_hasCombat && (!KJ_unkillable), "the combat feature is in the set and he can die"] call kjChk
if (isNil "KJD_killLive") then { KJD_killLive = true }
[("kill" in KJD_plan) && (!KJD_killLive), "the take ends with the kill beat, staged (the take 'staged')"] call kjChk
triSetView ("street" call kjView)
triSimFrames 8
triScreenshot "00_warmup_discard"

// ---- the beats before it, with the combat feature in the set -------------------
triSimUntil { ("fire" in KJD_done) || (time > kjT0 + 330) }
logInfo format ["KJK fire: done %1, hurt count %2, hurt %3, wound stage %4, health %5, states %6", KJD_done, KJ_hurtCount, KJ_hurt, call KJ_fnWound, KJ_woundHealth, KJT_states]
["fire" in KJD_done, "the beats up to the armour's fire played"] call kjChk
[("flinch" in KJT_states) && (KJ_hurtCount >= 1), "a HEAT round through the combat feature made him flinch"] call kjChk
[(KJ_hurt select 4) in [kjdTank, kjdTank2, kjdBmp], "the combat feature's hit signal names the shooter"] call kjChk
[alive kaiju, "three HEAT rounds do not kill him"] call kjChk
triSimUntil { ("spent" in KJD_done) || (time > kjT0 + 460) }
[("beam" in KJT_states) && ("spent" in KJD_done), "the beam and the spent beat played"] call kjChk
logInfo format ["KJK before the kill: health %1, stage %2, alive %3, state %4", KJ_woundHealth, call KJ_fnWound, alive kaiju, KJ_state]

// ---- the kill ------------------------------------------------------------------------
triSimUntil { (KJD_step == "kill") || (time > kjT0 + 480) }
if (true) then { kjT1 = time }
triSimUntil { time > kjT1 + 4 }
triSetView ("north" call kjView)
triSimFrames 6
triScreenshot "12_the_reserve_platoon_fires"
logInfo format ["KJK volley: health %1, stage %2, pace %3, fire log entries %4, furnace %5", KJ_woundHealth, call KJ_fnWound, call KJ_fnPace, count KJD_fireLog, KJ_furnaceNow]
[(call KJ_fnWound) >= 3, "kill: he is badly hurt (wound stage 3 or more) under the volleys"] call kjChk
triSimUntil { KJ_dead || (time > kjT1 + 160) }
if (true) then { kjT2 = time }
[KJ_dead, "kill: the heavy force killed him"] call kjChk
logInfo format ["KJK dead: %1 s into the beat, death phase %2, state %3, brain on %4", time - kjT1, KJ_deathPhase, KJ_state, KJ_brainOn]
triSimUntil { (KJ_deathPhase == "flare") || (time > kjT2 + 30) }
triSimFrames 90
triSetView ("square" call kjView)
triSimFrames 6
triScreenshot "12b_dying_the_furnace_flares"
triSimUntil { (KJ_deathPhase == "fall") || (time > kjT2 + 60) }
triSimFrames 150
triSetView ("north" call kjView)
triSimFrames 6
triScreenshot "12c_he_falls"
triSimUntil { (KJ_deathPhase == "body") || (time > kjT2 + 120) }
triSimFrames 120
triSetView ("north" call kjView)
triSimFrames 6
triScreenshot "12d_the_body_from_the_north"
triSetView ("west" call kjView)
triSimFrames 6
triScreenshot "12e_the_body_from_the_road_in"
triSetView ("high" call kjView)
triSimFrames 6
triScreenshot "12f_the_body_from_above"
logInfo format ["KJK end: death log %1, death stats %2, alive %3, null %4, state %5, brain on %6, furnace on %7, presence on %8, crush on %9, ready %10, lights %11", KJ_deathLog, KJ_deathStats, alive kaiju, isNull kaiju, KJ_state, KJ_brainOn, KJ_furnaceOn, KJ_presenceOn, KJ_crushOn, KJ_ready, if (KJ_native) then { kjLightCount kaiju } else { -1 }]
logInfo format ["KJK end: done %1, states %2, combat stats %3, by ammo %4, reserve alive %5, jumps %6", KJD_done, KJT_states, KJ_combatStats, KJ_combatByAmmo, { alive _x } count [kjdR1, kjdR2, kjdR3, kjdR4], KJT_jumps]
[KJ_deathPhase == "body", "kill: the death played to its end"] call kjChk
[({ (_x select 0) in ["stagger", "flare", "vent", "dark", "fall", "down", "body"] } count KJ_deathLog) >= 6, "kill: stagger, flare, vent, dark, fall, down and body all came"] call kjChk
[!(alive kaiju), "kill: he is dead"] call kjChk
[!(isNull kaiju), "kill: the body stays"] call kjChk
[(KJ_state == "dead") && (!KJ_brainOn), "kill: the brain has stopped (no walking corpse)"] call kjChk
[(!KJ_furnaceOn) && (!KJ_presenceOn) && (!KJ_crushOn), "kill: the furnace loop, the drone and the crush loop have stopped"] call kjChk
// (a native command may only stand inside a block the stock binary never runs)
kjLights = -1
if (KJ_native) then { kjLights = kjLightCount kaiju }
[(!KJ_native) || (kjLights == 0), "kill: the furnace light is out"] call kjChk
[({ alive _x } count [kjdR1, kjdR2, kjdR3, kjdR4]) == 4, "kill: the armour that killed him lives"] call kjChk
["kill" in KJD_done, "kill: the beat ended by itself"] call kjChk
logInfo format ["KJK RESULT: %1 checks failed: %2", count kjFails, kjFails]
// the engine's light and emitter must be gone before the game exits (integration NOTES, engine request 1)
if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false; KJ_combatOn = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
// The verdict is the RESULT line above, read by run-kj.ps1 from the game's log (not a failing triAssert).
triEndTest
