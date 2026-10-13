// ============================================================================
//  LoBo #9 remaining runtime items (repair 2026-10-11.4 runtime plan R3,
//  installed @LoBo only). Repair .3 made KioGlowFire (and M1Smoker, M60Smoker,
//  Smoker) an invisible vanilla Fire; repair3/lobo_wreck_glow proved the glow
//  objects exist. This probe covers what that one did not, on the LoBoEghelo
//  Mi-8 wreck (\LoBoEghelo\Engine\fire.sqs, installed bytes read 2026-10-12):
//    S   isolated glow next to a vanilla Fire control, noon and 23:00 shots.
//    D1  "Put out fire" on the isolated glow: offered, works, "Light fire" after.
//    A   t0..t0+70 s: player 60 m away. The script spawns mortar shells while
//        _lifeTick > 550 and bullets while > 360 (60 s at 0.25 s a tick plus
//        frame overshoot), so the seated and on-foot phases start at t0+70,
//        8 s later than SPEC 10 says, to clear the bullet window.
//    B   t0+70..t0+98: player seated in a Jeep 7 m from the wreck: no burn.
//    C   t0+98..t0+133: on foot at the same spot: +0.04 per step, at most
//        floor(L/10)+2 steps, steps after the second at least 9.5 s apart.
//    D2  t0+133..t0+143: 3.5 m from the glow, "Put out fire" (inflame false as
//        a logged fallback). Script particles and say lines must continue.
//    E   player 60 m away; the glow is deleted 150 to 200 s after t0.
//  t0 = mission time when the KioGlowFire first exists within 30 m.
//  Damage is allowed on the player (the burn uses setdammage); he is captive.
//  Log checks (eng_lookout3 per step, objects\fire say count, no Load failed,
//  no script error) are in repair4/check_logs.py r3. Waits are chunked to stay
//  under assert_timeout, which bounds every triSimUntil in wall-clock time.
// ============================================================================
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
setViewDistance 1500
showCinemaBorder false
player setCaptive true
triAssertEq [damage player, 0]
wbCount = {wbCntN = 0; {if ((typeOf _x) == (_this select 1)) then {wbCntN = wbCntN + 1}} forEach (nearestObjects [_this select 0, [], 30]); wbCntN}

// --- S: isolated glow (left) and vanilla Fire control 4 m east (right) ---
wbGlow = "KioGlowFire" createVehicle [8000, 9560, 0]
triAssertEq [typeOf wbGlow, "KioGlowFire"]
wbCtl = "Fire" createVehicle [8004, 9560, 0]
triAssertEq [typeOf wbCtl, "Fire"]
call {wbGlow setPos [8000, 9560, 0]; wbCtl setPos [8004, 9560, 0]; wbGlow inflame true; wbCtl inflame true}
triSimFrames 60
triAssertEq [inflamed wbGlow, true]
triAssertEq [inflamed wbCtl, true]
logInfo format ["LOBO_R4_BURN case=S glow=%1 control=%2 gap=%3 glowInflamed=%4 controlInflamed=%5", typeOf wbGlow, typeOf wbCtl, wbGlow distance wbCtl, inflamed wbGlow, inflamed wbCtl]
wbP = getPosASL wbGlow
triSetView [(wbP select 0) + 2, (wbP select 2) + 0.6, (wbP select 1) - 4, 0, -0.08, 1]
triSimFrames 5
triScreenshot "s_glow_left_fire_right_noon"
setDate [1985, 6, 15, 23, 0]
triSimFrames 60
triScreenshot "s_glow_left_fire_right_night"
triClearView

// --- D1: "Put out fire" on the isolated glow (control removed first) ---
deleteVehicle wbCtl
call {player setPos [8000, 9556.5, 0]; player setDir 0}
triSimFrames 30
logInfo format ["LOBO_R4_BURN case=D1 before playerToGlow=%1 menu=%2", player distance wbGlow, triActionMenuText]
triAssertIncludes [triActionMenuText, "Put out fire"]
player action ["FIRE PUT DOWN", wbGlow]
triSimFrames 30
triAssertEq [inflamed wbGlow, false]
triAssertIncludes [triActionMenuText, "Light fire"]
logInfo format ["LOBO_R4_BURN case=D1 after inflamed=%1 menu=%2 playerDamage=%3", inflamed wbGlow, triActionMenuText, damage player]
deleteVehicle wbGlow
triSimFrames 5

// --- Wreck: empty Mi-8 killed through its native handler, at night ---
call {player setPos [8000, 9760, 0]; player setDir 0}
wbWreck = "LoBo_Mi8_egy" createVehicle [8000, 9820, 0]
triAssertEq [typeOf wbWreck, "LoBo_Mi8_egy"]
call {wbWreck setDir 0; wbWreck setPos [8000, 9820, 0]}
triSimFrames 10
triAssertEq [[getPos wbWreck, "KioGlowFire"] call wbCount, 0]
call {wbDmg0 = damage player; wbWreck setDammage 1; wbTDead = time}
triSimUntil { ([getPos wbWreck, "KioGlowFire"] call wbCount) == 1 }
call {wbT0 = time; wbFire = nearestObject [getPos wbWreck, "KioGlowFire"]; logInfo format ["LOBO_R4_BURN_T0 t0=%1 sinceKill=%2 glow=%3 glowToWreck=%4 inflamed=%5 playerToWreck=%6 playerDamage=%7", wbT0, wbT0 - wbTDead, typeOf wbFire, wbFire distance wbWreck, inflamed wbFire, player distance wbWreck, damage player]}
triAssertEq [typeOf wbFire, "KioGlowFire"]

// --- A: stay 60 m away through the random-explosion windows ---
triSimUntil { time > wbT0 + 20 }
wbP = getPosASL wbWreck
triSetView [(wbP select 0) - 8.5, (wbP select 2) + 3, (wbP select 1) - 8.5, 8.5, -2.5, 8.5]
triSimFrames 5
triScreenshot "a_wreck_glow_night_12m"
triClearView
logInfo format ["LOBO_R4_BURN case=A t=%1 playerDamage=%2 playerToWreck=%3 glowInflamed=%4", time - wbT0, damage player, player distance wbWreck, inflamed wbFire]
triSimUntil { time > wbT0 + 45 }
triSimUntil { time > wbT0 + 70 }

// --- B: seated in a Jeep 7 m from the wreck: no burn for 28 s ---
wbJeep = "Jeep" createVehicle [8007, 9820, 0]
triAssertEq [typeOf wbJeep, "Jeep"]
call {wbJeep setDir 0; wbJeep setPos [8007, 9820, 0]; player moveInDriver wbJeep}
triSimUntil { vehicle player == wbJeep }
triSimFrames 10
call {wbB0 = time; wbBP = damage player; wbBJ = damage wbJeep; wbBMaxP = wbBP; wbBMaxJ = wbBJ; wbBMaxDist = player distance wbWreck; wbBOut = false}
logInfo format ["LOBO_R4_BURN case=B start t=%1 seated=%2 playerToWreck=%3 playerDamage=%4 jeepDamage=%5", wbB0 - wbT0, vehicle player == wbJeep, player distance wbWreck, wbBP, wbBJ]
wbBTick = {if ((player distance wbWreck) > wbBMaxDist) then {wbBMaxDist = player distance wbWreck}; if ((damage player) > wbBMaxP) then {wbBMaxP = damage player}; if ((damage wbJeep) > wbBMaxJ) then {wbBMaxJ = damage wbJeep}; if (vehicle player != wbJeep) then {wbBOut = true}; true}
triSimUntil { ([] call wbBTick) && (time > wbB0 + 14) }
triSimUntil { ([] call wbBTick) && (time > wbB0 + 28) }
logInfo format ["LOBO_R4_BURN case=B end t=%1 seatedSeconds=%2 leftSeat=%3 maxPlayerToWreck=%4 playerDamage=%5->%6 jeepDamage=%7->%8", time - wbT0, time - wbB0, wbBOut, wbBMaxDist, wbBP, wbBMaxP, wbBJ, wbBMaxJ]
triAssertEq [wbBOut, false]
triAssertLt [wbBMaxDist, 10]
triAssertEq [wbBMaxP, wbBP]
triAssertEq [wbBMaxJ, wbBJ]

// --- C: on foot at the same spot, every damage step sampled ---
call {wbC0 = -1; wbLast = damage player; wbSteps = []; wbStepT = []; wbCMaxDist = 0; player action ["GETOUT", wbJeep]}
wbCTick = {_d = damage player; if (_d > wbLast + 0.0001) then {wbSteps = wbSteps + [_d - wbLast]; wbStepT = wbStepT + [time]; wbLast = _d}; if ((wbC0 < 0) && (vehicle player == player)) then {wbC0 = time}; if ((player distance wbWreck) > wbCMaxDist) then {wbCMaxDist = player distance wbWreck}; true}
triSimUntil { ([] call wbCTick) && (vehicle player == player) }
call {[] call wbCTick; deleteVehicle wbJeep; player setPos [8006.5, 9820, 0]; player setDir 270}
logInfo format ["LOBO_R4_BURN case=C start t=%1 onFoot=%2 playerToWreck=%3 playerDamage=%4 stepsSoFar=%5", wbC0 - wbT0, vehicle player == player, player distance wbWreck, damage player, count wbStepT]
triSimUntil { ([] call wbCTick) && (((count wbStepT) > 0) || (time > wbT0 + 133)) }
triScreenshot "c_first_burn_medic"
call {wbFS = time; if ((count wbStepT) > 0) then {wbFS = wbStepT select 0}; logInfo format ["LOBO_R4_BURN case=C firstStep t=%1 shotAfterStep=%2 playerDamage=%3 steps=%4", time - wbT0, time - wbFS, damage player, count wbStepT]}
triSimUntil { ([] call wbCTick) && (time > wbT0 + 116) }
triSimUntil { ([] call wbCTick) && (time > wbT0 + 133) }
call {[] call wbCTick; wbL = time - wbC0; wbNC = count wbStepT; wbBad = 0; {if ((abs (_x - 0.04)) > 0.001) then {wbBad = wbBad + 1}} forEach wbSteps; wbMinGap = 999; wbI = 0; {if (wbI >= 2) then {if ((_x - (wbStepT select (wbI - 1))) < wbMinGap) then {wbMinGap = _x - (wbStepT select (wbI - 1))}}; wbI = wbI + 1} forEach wbStepT; wbRel = []; {wbRel = wbRel + [_x - wbT0]} forEach wbStepT}
logInfo format ["LOBO_R4_BURN case=C end onFootSeconds=%1 steps=%2 deltas=%3 stepTimesSinceT0=%4 badDeltas=%5 minGapFromSecondStep=%6 maxPlayerToWreck=%7 playerDamage=%8", wbL, wbNC, wbSteps, wbRel, wbBad, wbMinGap, wbCMaxDist, damage player]
triAssertLt [wbCMaxDist, 10]
triAssertGe [wbNC, 2]
triAssertEq [wbBad, 0]
triAssertLe [(wbNC - 2) * 10, wbL]
triAssertGe [wbMinGap, 9.5]

// --- D2: "Put out fire" at the wreck glow (3.5 m, facing it) ---
call {[] call wbCTick; wbFP = getPos wbFire; player setPos [(wbFP select 0) + 3.5, wbFP select 1, 0]; player setDir 270; wbD2T = time}
triSimUntil { ([] call wbCTick) && (((triAssertIncludes [triActionMenuText, "Put out fire"]) == "OK") || (time > wbD2T + 3)) }
call {[] call wbCTick; wbD2Offered = (triAssertIncludes [triActionMenuText, "Put out fire"]) == "OK"; wbD2Menu = triActionMenuText; wbD2Dist = player distance wbFire}
logInfo format ["LOBO_R4_BURN case=D2 offered=%1 playerToGlow=%2 playerToWreck=%3 menu=%4", wbD2Offered, wbD2Dist, player distance wbWreck, wbD2Menu]
player action ["FIRE PUT DOWN", wbFire]
triSimUntil { ([] call wbCTick) && ((not (inflamed wbFire)) || (time > wbD2T + 6)) }
call {[] call wbCTick; wbD2Fallback = inflamed wbFire; if (wbD2Fallback) then {wbFire inflame false}}
logInfo format ["LOBO_R4_BURN case=D2 actionWorked=%1 fallbackInflameFalse=%2", not wbD2Fallback, wbD2Fallback]
triSimUntil { ([] call wbCTick) && (time > wbD2T + 6) }
wbP = getPosASL wbWreck
triSetView [(wbP select 0) - 8.5, (wbP select 2) + 3, (wbP select 1) - 8.5, 8.5, -2.5, 8.5]
triSimFrames 5
triScreenshot "d2_after_put_out_12m"
triClearView
triSimUntil { ([] call wbCTick) && (time > wbD2T + 10) }
call {[] call wbCTick; logInfo format ["LOBO_R4_BURN_D2_END t=%1 offered=%2 fallbackInflameFalse=%3 inflamed=%4 stepsC=%5 stepsTotal=%6 playerDamage=%7", time - wbT0, wbD2Offered, wbD2Fallback, inflamed wbFire, wbNC, count wbStepT, damage player]}
triAssertEq [inflamed wbFire, false]
triAssertEq [isNull wbFire, false]

// --- E: player 60 m away; the script deletes the glow 150 to 200 s after t0 ---
call {player setPos [8000, 9760, 0]; player setDir 0}
triSimUntil { (([getPos wbWreck, "KioGlowFire"] call wbCount) == 0) || (time > wbT0 + 175) }
triSimUntil { (([getPos wbWreck, "KioGlowFire"] call wbCount) == 0) || (time > wbT0 + 210) }
call {wbTDel = time - wbT0; logInfo format ["LOBO_R4_BURN_CLEANUP sinceT0=%1 glowCount=%2 glowNull=%3 wreckNull=%4 wreckAlive=%5 playerDamage=%6", wbTDel, [getPos wbWreck, "KioGlowFire"] call wbCount, isNull wbFire, isNull wbWreck, alive wbWreck, damage player]}
triAssertEq [[getPos wbWreck, "KioGlowFire"] call wbCount, 0]
triAssertGe [wbTDel, 150]
triAssertLe [wbTDel, 200]
triAssertEq [isNull wbWreck, false]
logInfo "LOBO_R4_BURN_DONE"
triSimFrames 30
triEndTest
