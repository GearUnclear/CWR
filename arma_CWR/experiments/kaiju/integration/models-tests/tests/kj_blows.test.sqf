// ============================================================================
//  Dedushka integration: the blows, driven from the brain (KJ_brainOrder) and
//  in step with the clips; a stomp he chooses by himself; unkillable; dead.
//  Soft checks, collected and asserted once at the end (KJB lines in the log).
//  The creature class is KJD_class (the take: run-kj.ps1 -Params proxy or hound; default UDDedushka).
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4900, 4090, 0]
setDate [1985, 6, 10, 16, 0]
0 setOvercast 0.25
0 setFog 0
setViewDistance 2000
showCinemaBorder false
call compile preprocessFile "kjd_params.sqf"
if (isNil "KJD_class") then { KJD_class = "UDDedushka" }
kjbC = createCenter civilian
kjbG = createGroup civilian
kjbG2 = createGroup civilian
kaiju = objNull
kjbAt = [4900, 4004, 0]
KJD_class createUnit [kjbAt, kjbG, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
if (true) then { kaiju setDir 90; kaiju setPos kjbAt; kaiju setCaptive true; KJ_brainAuto = false; KJ_ready = false; KJT_on = true; [kaiju, "idle"] exec "kj\start.sqs" }
triSimUntil { KJ_ready }
triSimFrames 120
[kaiju] exec "kj\brain\watch.sqs"
kjFails = []
kjChk = { if (!(_this select 0)) then { kjFails = kjFails + [_this select 1]; logInfo format ["KJB CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJB ok: %1", _this select 1] } }
KJD_probe = "HeliHEmpty" createVehicle [4900, 4100, 0]
// a camera object takes the view, so no crosshair or HUD is in the frames (triSetView then places the view)
kjCamObj = "camera" camCreate [4960, 3960, 3]
kjCamObj cameraEffect ["internal", "back"]
showCinemaBorder false
kjbView = { private ["_g"]; KJD_probe setPos [_this select 0, _this select 1, 0]; _g = ((getPosASL KJD_probe) select 2) + (_this select 2); [_this select 0, _g, _this select 1, ((getPosASL kaiju) select 0) - (_this select 0), ((getPosASL kaiju) select 2) + (_this select 3) - _g, ((getPosASL kaiju) select 1) - (_this select 1)] }
// a point in his frame, on the ground: [forward, right] -> [x, y, 0]
kjbAtFR = { [kaiju, _this select 0, _this select 1] call KJC_fnWorld }
logInfo format ["KJB start: creature %1, native %2, height %3, stomp angle %4, stomp secs %5, swat secs %6, timing %7", typeOf kaiju, KJ_native, KJ_heightM, KJB_stompAng, KJB_stompSecs, KJB_swatSecs, KJ_timingFile]
triSetView ([4960, 3960, 1.7, 14] call kjbView)
triSimFrames 6
triScreenshot "00_warmup_discard"

// ---- 0. nothing on the ground can shove him. The crush loop (which clears his feet) is stopped,
// ---- then a man is stood inside his right leg and a live tank inside his left one. On a creature
// ---- whose collision shape reaches the ground the engine pushes him metres aside for this.
if (true) then { KJ_crushOn = false; kjbT = time; kjbP0 = getPos kaiju }
triSimUntil { time > kjbT + 1.5 }
kjbShMan = objNull
"Civilian" createUnit [[4705, 4320, 0], kjbG2, "kjbShMan = this", 0.5, "PRIVATE"]
kjbShTank = "T80" createVehicle [4700, 4330, 0]
triSimFrames 20
if (true) then { kjbShMan disableAI "MOVE"; kjbShMan stop true; kjbShMan setPos ([1, 5] call kjbAtFR); kjbShTank setPos ([1, -5] call kjbAtFR); kjbT = time }
triSimUntil { time > kjbT + 5 }
logInfo format ["KJB shove: he moved %1 m in 5 s with a man and a tank standing in his legs (man at %2, tank at %3, him at %4)", [getPos kaiju, kjbP0] call KJB_fnDist, getPos kjbShMan, getPos kjbShTank, getPos kaiju]
[([getPos kaiju, kjbP0] call KJB_fnDist) < 0.3, "shove: a man and a tank standing in his legs do not move him (the collision shape is lifted)"] call kjChk
triSetView ([((getPos kaiju) select 0) + 50, ((getPos kaiju) select 1) - 40, 20, 6] call kjbView)
triSimFrames 4
triScreenshot "b00_a_man_and_a_tank_in_his_legs"
deleteVehicle kjbShMan
deleteVehicle kjbShTank
if (true) then { kaiju setPos kjbAt; kaiju setDir 90; KJ_crushOn = true; [kaiju] exec "kj\crush\loop.sqs" }
triSimFrames 90

// ---- 1. the stomp, ordered: a tank ahead of him and to his right, men round it ----------------
kjbTank = "T80" createVehicle [4700, 4200, 0]
kjbMen = []
if (true) then { kjbTank setPos ([12, 3] call kjbAtFR); kjbTank setDir 0; kjbTp = getPos kjbTank }
{ "Civilian" createUnit [[4700 + 5 * _x, 4150, 0], kjbG2, "kjbMen = kjbMen + [this]", 0.5, "PRIVATE"] } forEach [0, 1, 2]
triSimFrames 20
if (true) then { { _x disableAI "MOVE"; _x stop true; _x allowFleeing 0 } forEach kjbMen; (kjbMen select 0) setPos [(kjbTp select 0) + 5, (kjbTp select 1) - 2, 0]; (kjbMen select 1) setPos [(kjbTp select 0) + 22, (kjbTp select 1) - 6, 0]; (kjbMen select 2) setPos [(kjbTp select 0) + 70, (kjbTp select 1), 0] }
triSimFrames 60
if (true) then { kjbT = time; kjbN = KJ_stompCount; kjbP0 = getPos kaiju; KJ_brainOrder = ["stomp", kjbTank] }
triSimUntil { (KJ_stomping) || (time > kjbT + 40) }
if (true) then { kjbT1 = time }
[KJ_stomping, "stomp: the brain turned him and started the stomp"] call kjChk
logInfo format ["KJB stomp wind-up: %1 s after the order, facing %2 (the tank bears %3), move %4", time - kjbT, getDir kaiju, [getPos kaiju, kjbTp] call KJB_fnBear, getMove kaiju]
triSimUntil { time > kjbT1 + 4.4 }
triSetView ([((getPos kaiju) select 0) + 55, ((getPos kaiju) select 1) - 45, 22, 10] call kjbView)
triSimFrames 4
triScreenshot "b01_stomp_the_foot_is_up"
triSimUntil { (KJ_stompCount > kjbN) || (time > kjbT1 + 14) }
logInfo format ["KJB stomp: landed %1 s after the clip started (clip says %2), result %3, where %4, tank was at %5, footfall %6", time - kjbT1, if (isNil "KJ_animStompTime") then { -1 } else { KJ_animStompTime }, KJ_stompResult, KJ_stompPos, kjbTp, KJ_footfall]
triSimFrames 20
triScreenshot "b02_stomp_the_blow"
[KJ_stompCount == (kjbN + 1), "stomp: the blow landed"] call kjChk
[KJ_stompResult == "hit", "stomp: on the tank (the foot of the clip came down on it)"] call kjChk
[(isNil "KJ_animStompTime") || ((abs ((time - kjbT1) - KJ_animStompTime)) < 1.2), "stomp: the blow came when the foot of the clip came down"] call kjChk
[(isNull kjbTank) || (!(alive kjbTank)), "stomp: the tank is flattened"] call kjChk
[!(alive (kjbMen select 0)), "stomp: the man in the kill zone is dead"] call kjChk
[(alive (kjbMen select 1)) && ((getDammage (kjbMen select 1)) > 0.05), "stomp: the man in the ring is hurt and alive"] call kjChk
[(alive (kjbMen select 2)) && ((getDammage (kjbMen select 2)) < 0.01), "stomp: the far man is untouched"] call kjChk
["dedstomp" in KJT_moves, "stomp: the stomp clip played"] call kjChk
[(KJ_footfall select 3) == "S", "stomp: raised as a stomp footfall (harder dust, thump and shake)"] call kjChk
triSimUntil { ((KJ_state == "idle") && (time > kjbT1 + 6)) || (time > kjbT1 + 30) }
[([getPos kaiju, kjbP0] call KJB_fnDist) < 1.5, "stomp: he stood his ground"] call kjChk
logInfo format ["KJB stomp end: man 0 %1 m from where he stood (thrown), states %2", [getPos (kjbMen select 0), [(kjbTp select 0) + 8, (kjbTp select 1) - 2, 0]] call KJB_fnDist, KJT_states]

// ---- 2. the kick: a tank on the ground in reach of his feet, ordered as a swat ----------------
kjbTank2 = "T80" createVehicle [4700, 4230, 0]
if (true) then { kjbTank2 setPos ([14, -2] call kjbAtFR); kjbTp = getPos kjbTank2 }
triSimFrames 60
if (true) then { kjbT = time; kjbN = KJ_swatCount; KJ_brainOrder = ["swat", kjbTank2] }
triSimUntil { (KJ_swatCount > kjbN) || (time > kjbT + 40) }
triSimFrames 90
logInfo format ["KJB kick: result %1 how %2, wound %3 s, tank damage %4, moved %5 m, move list %6", KJ_swatResult, KJ_swatHow, KJ_swatWind, getDammage kjbTank2, [getPos kjbTank2, kjbTp] call KJB_fnDist, KJT_moves]
[(KJ_swatResult == "hit") && (KJ_swatHow == "kick"), "kick: a tank at his feet is kicked"] call kjChk
[(getDammage kjbTank2) >= 0.79, "kick: the tank is crippled"] call kjChk
[([getPos kjbTank2, kjbTp] call KJB_fnDist) > 3, "kick: the tank was thrown aside"] call kjChk
triSimUntil { ((KJ_state == "idle") && (time > kjbT + 6)) || (time > kjbT + 50) }
deleteVehicle kjbTank2

// ---- 3. the swat: an empty helicopter hung where his right arm sweeps -------------------------
kjbHeli = "Mi17" createVehicle [4700, 4260, 0]
// clear of his body (a helicopter that touches his arm is wrecked by the engine before he moves): out
// at the end of his reach, where the hand of the clip passes
kjbHp = [14, 27] call kjbAtFR
// (the kick a moment ago left KJ_swatResult at "hit", which is what lets the helicopter go)
if (true) then { KJ_swatResult = ""; kjbHp = [kjbHp select 0, kjbHp select 1, 0.84 * KJ_heightM]; [kjbHeli, kjbHp] exec "lab_hold.sqs" }
triSimFrames 60
logInfo format ["KJB swat staging: helicopter alive %1 at %2, reach %3", alive kjbHeli, getPos kjbHeli, [kaiju, kjbHeli, 1] call KJC_fnReach]
if (true) then { kjbT = time; kjbN = KJ_swatCount; KJ_brainOrder = ["swat", kjbHeli] }
triSimUntil { (KJ_swatting) || (time > kjbT + 30) }
if (true) then { kjbT1 = time }
triSimUntil { time > kjbT1 + 3.6 }
triSetView ([((getPos kaiju) select 0) + 80, ((getPos kaiju) select 1) + 30, 34, 28] call kjbView)
triSimFrames 4
triScreenshot "b03_swat_the_arm_comes_round"
triSimUntil { (KJ_swatCount > kjbN) || (KJ_swatResult == "miss") || (time > kjbT1 + 14) }
triSimFrames 6
triScreenshot "b04_swat_the_blow"
if (true) then { kjbT2 = time }
triSimUntil { time > kjbT2 + 3 }
logInfo format ["KJB swat: result %1 how %2, wound %3 s (landed %4 s after the swing began), helicopter alive %5, %6 m from where it hung, height %7", KJ_swatResult, KJ_swatHow, KJ_swatWind, kjbT2 - kjbT1, alive kjbHeli, [getPos kjbHeli, kjbHp] call KJB_fnDist, (getPos kjbHeli) select 2]
[(KJ_swatResult == "hit") && (KJ_swatHow == "arm"), "swat: the helicopter in the sweep of his arm is struck"] call kjChk
[!(alive kjbHeli), "swat: it is destroyed"] call kjChk
[([getPos kjbHeli, kjbHp] call KJB_fnDist) > 15, "swat: it was thrown"] call kjChk
["dedswat" in KJT_moves, "swat: the swat clip played"] call kjChk
[(isNil "KJ_animSwatPath") || ((KJ_swatWind > 2.8) && (KJ_swatWind < 6)), "swat: the blow was timed to the hand of the clip"] call kjChk
triSimUntil { ((KJ_state == "idle") && (time > kjbT1 + 6)) || (time > kjbT1 + 40) }
KJL_hold = false

// ---- 4. a stomp he chooses himself: a crewed tank rolls up in front of him ----------------------
kjbTank3 = "T80" createVehicle [4700, 4290, 0]
kjbDrv = objNull
"Civilian" createUnit [[4705, 4290, 0], kjbG2, "kjbDrv = this", 0.5, "PRIVATE"]
triSimFrames 20
kjbDrv moveInDriver kjbTank3
// crewed, but it must stay where it is put: no fuel
kjbTank3 setFuel 0
triSimFrames 30
// (the swat a moment ago must not count as "a blow less than twenty seconds ago", or he wanders off first)
if (true) then { kjbTank3 setPos ([13, 1] call kjbAtFR); KJB_lastBlow = -1000; kjbT = time; kjbN = KJ_stompCount; KJ_brainOrder = ["auto", true] }
triSimUntil { (KJ_state == "stomp") || (time > kjbT + 45) }
logInfo format ["KJB auto: state %1 phase %2, %3 s after he was left to himself, crew %4", KJ_state, KJ_brainPhase, time - kjbT, count (crew kjbTank3)]
[KJ_state == "stomp", "auto: left to himself he stomps the crewed tank at his feet"] call kjChk
triSimUntil { (KJ_stompCount > kjbN) || (time > kjbT + 80) }
triSimFrames 60
logInfo format ["KJB auto stomp: result %1 at %2, the tank at %3 (%4 m from the blow), its damage %5, null %6", KJ_stompResult, KJ_stompPos, getPos kjbTank3, [getPos kjbTank3, KJ_stompPos] call KJB_fnDist, getDammage kjbTank3, isNull kjbTank3]
[KJ_stompCount > kjbN, "auto: the stomp landed"] call kjChk
[(isNull kjbTank3) || (!(alive kjbTank3)) || ((getDammage kjbTank3) >= 0.79), "auto: the tank is flattened or crippled"] call kjChk
KJ_brainOrder = ["auto", false]
triSimUntil { ((KJ_state == "idle") && (KJB_mv == "stand")) || (time > kjbT + 120) }
KJ_brainOrder = ["idle"]
triSimFrames 60

// ---- 5. a killing blow by script. Unkillable (no combat feature in the set): it is undone and the
// ---- brain goes on. Killable (the combat feature): it kills him, and section 6 finds everything stopped.
if (true) then { kjbR = KJ_revived; kaiju setDammage 1 }
triSimFrames 90
logInfo format ["KJB killing blow: alive %1, revived %2 -> %3, damage %4, brain on %5, state %6, unkillable %7", alive kaiju, kjbR, KJ_revived, getDammage kaiju, KJ_brainOn, KJ_state, KJ_unkillable]
[(!KJ_unkillable) || ((alive kaiju) && (KJ_revived > kjbR)), "unkillable: a killing blow is undone"] call kjChk
[(!KJ_unkillable) || (KJ_brainOn && (KJ_state != "dead")), "unkillable: the brain goes on after it"] call kjChk
if (true) then { kjbT = time; kjbP0 = getPos kaiju; KJ_brainOrder = ["drawn", [((getPos kaiju) select 0) + 40, (getPos kaiju) select 1, 0]] }
triSimUntil { (([getPos kaiju, kjbP0] call KJB_fnDist) > 10) || (!KJ_unkillable) || (time > kjbT + 60) }
[(!KJ_unkillable) || (([getPos kaiju, kjbP0] call KJB_fnDist) > 10), "unkillable: he walks afterwards"] call kjChk
[KJ_unkillable || (!(alive kaiju)), "killable (the combat feature is in the set): a script's killing blow kills him"] call kjChk
KJ_brainOrder = ["idle"]
triSimUntil { ((KJ_state == "idle") && (KJB_mv == "stand")) || (!KJ_unkillable) || (time > kjbT + 90) }

// ---- 6. dead: everything stops, cleanly ------------------------------------------------------------
if (true) then { KJ_unkillable = false; KJ_dead = true; kaiju setDammage 1; kjbT = time }
triSimUntil { time > kjbT + 2.5 }
logInfo format ["KJB dead: state %1, brain on %2, furnace on %3, presence on %4, crush on %5, ready %6, lights %7, alive %8, move %9", KJ_state, KJ_brainOn, KJ_furnaceOn, KJ_presenceOn, KJ_crushOn, KJ_ready, if (KJ_native) then { kjLightCount kaiju } else { -1 }, alive kaiju, getMove kaiju]
[(KJ_state == "dead") && (!KJ_brainOn), "dead: the brain has stopped"] call kjChk
[!KJ_furnaceOn, "dead: the furnace loop has stopped"] call kjChk
// (a native command may only stand inside a block the stock binary never runs)
kjLights = -1
if (KJ_native) then { kjLights = kjLightCount kaiju }
[(!KJ_native) || (kjLights == 0), "dead: the furnace light is out"] call kjChk
[(!KJ_presenceOn) && (!KJ_crushOn), "dead: no drone, no crush loop"] call kjChk
[!(alive kaiju), "dead: he stays dead (nothing revives him)"] call kjChk
triSetView ([((getPos kaiju) select 0) + 70, ((getPos kaiju) select 1) - 60, 30, 6] call kjbView)
triSimFrames 240
triScreenshot "b05_dead"
logInfo format ["KJB end: jumps %1, pushes %2, crush %3", KJT_jumps, KJ_crushPushes, KJ_crushStats]
logInfo format ["KJB RESULT: %1 checks failed: %2", count kjFails, kjFails]
// the engine's light and emitter must be gone before the game exits (a kjLight alive at exit crashes the exit: integration NOTES, engine request 1)
if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
// The verdict is the RESULT line above, read by run-kj.ps1 from the game's log. It is NOT a failing
// triAssert: a game with a live creature that fails an assert stops answering the harness and holds the
// machine's one game slot until the test's time limit (seen here and by the brain feature).
triEndTest
