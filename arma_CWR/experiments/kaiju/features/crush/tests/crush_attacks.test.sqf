// ============================================================================
//  Dedushka CRUSH: the stomp, the kick, the swat and being shot (experiment,
//  kept out of tests/integration). Open flat ground south of Morton (Everon).
//  Quiet: nobody is armed against anybody, nobody fires; "being shot" is tested
//  with scripted damage only. Needs the crush variant of the creature addon
//  (@kj_crush). Run it through run-crush.ps1.
//
//  1. A stomp on bare ground among men: the one in the kill zone dies, the two
//     in the shock ring are thrown down and hurt but live, the one outside the
//     ring is untouched.
//  2. A stomp on a T-80: the tank is flattened, a jeep in the ring is wrecked,
//     an APC in the ring survives, a jeep outside the ring is untouched.
//  3. A stomp ordered 100 m away lands at the end of his reach ("short").
//  4. The kick: a T-80 beside his foot is kicked aside and crippled.
//  5. The swat: a helicopter with its engine running, carried past his
//     shoulder by the test, is struck out of the air when it comes within
//     reach of his arm; a jeep far out of reach is left alone ("out").
//  6. Being shot: scripted damage is healed within half a second and raises
//     KJ_hurt; a remembered heavy shot gives the hit its direction; a rifle
//     round is not remembered; a killing blow is undone; forty blows in four
//     seconds do not add up; he walks afterwards.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [5200, 4160, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.2
0 setFog 0
setViewDistance 1500
showCinemaBorder false
caC = createCenter civilian
caG = createGroup civilian
caG2 = createGroup civilian
ded = objNull
"UDDedushka" createUnit [[5075, 3882, 0], caG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
if (true) then { ded setCaptive true; ded setBehaviour "CARELESS"; ded allowFleeing 0; ded setDir 180; ded setPos [5075, 3882, 0]; ded stop true }
call compile preprocessFile "kj\crush\init.sqf"
[ded] call KJC_fnSize
caCam = "camera" camCreate [5055, 3700, 30]
caCam cameraEffect ["internal", "back"]
if (true) then { caCam camSetTarget ded; caCam camSetFov 0.7; caCam camCommit 0 }
triSimFrames 30
[ded] exec "kj\crush\loop.sqs"
// caAt: [forward, right] in his frame -> world position. He faces south, so right is west.
// caSee: camera at [forward, right, up] in his frame, looking at [forward, right, up].
caAt = { [ded, _this select 0, _this select 1] call KJC_fnWorld }
caSee = { private ["_a", "_b", "_g"]; _a = [ded, _this select 0, _this select 1] call KJC_fnWorld; _b = [ded, _this select 3, _this select 4] call KJC_fnWorld; _g = (getPosASL ded) select 2; triSetView [_a select 0, _g + (_this select 2), _a select 1, (_b select 0) - (_a select 0), (_this select 5) - (_this select 2), (_b select 1) - (_a select 1)] }
// createVehicle will not put a vehicle near him (it looks for free room around his whole radius), so each is made far off and then set in place.
caPut = { private ["_v"]; _v = (_this select 0) createVehicle [5230, 4000, 0]; _v setPos ([_this select 1, _this select 2] call caAt); _v setDir 0; _v }
caJeepFar = ["UAZ", 13, -46] call caPut

// ---- 1. a stomp on bare ground among men ---------------------------------------------
caKill = objNull
caRing1 = objNull
caRing2 = objNull
caFar = objNull
"Civilian" createUnit [[13, 9] call caAt, caG2, "caKill = this", 0.5, "PRIVATE"]
"Civilian2" createUnit [[13, -18] call caAt, caG2, "caRing1 = this", 0.5, "PRIVATE"]
"Civilian3" createUnit [[39, 0] call caAt, caG2, "caRing2 = this", 0.5, "PRIVATE"]
"Civilian" createUnit [[-20, 46] call caAt, caG2, "caFar = this", 0.5, "PRIVATE"]
triSimFrames 60
// createUnit puts a man in his group's formation, not on the spot asked for: set each in place
if (true) then { caKill setPos ([13, 9] call caAt); caRing1 setPos ([13, -18] call caAt); caRing2 setPos ([39, 0] call caAt); caFar setPos ([-20, 46] call caAt) }
{ _x stop true } forEach [caKill, caRing1, caRing2, caFar]
[46, -30, 2.5, 12, 0, 9] call caSee
triSimFrames 6
triScreenshot "a00_discard"
triScreenshot "a01_stomp_before"
if (true) then { [ded, [13, 0] call caAt] exec "kj\crush\stomp.sqs"; caT0 = time }
triSimUntil { time > caT0 + KJC_STOMP_WIND + 0.1 }
triScreenshot "a02_stomp_blow"
triSimUntil { time > caT0 + KJC_STOMP_WIND + 0.45 }
triScreenshot "a03_stomp_ring"
triSimUntil { time > caT0 + KJC_STOMP_WIND + 1.2 }
triScreenshot "a04_stomp_dust"
triSimUntil { (not KJ_stomping) and (time > caT0 + 5) }
logInfo format ["CRUSH stomp men: result %1 ; kill alive %2 ; ring1 alive %3 dmg %4 move %5 ; ring2 alive %6 dmg %7 move %8 ; far dmg %9", KJ_stompResult, alive caKill, alive caRing1, getDammage caRing1, getMove caRing1, alive caRing2, getDammage caRing2, getMove caRing2, getDammage caFar]
triAssertEq [KJ_stompResult, "hit"]
triAssertEq [KJ_stompCount, 1]
triAssert [!(alive caKill)]
triAssert [alive caRing1]
triAssertGt [getDammage caRing1, 0.2]
triAssert [alive caRing2]
triAssertGt [getDammage caRing2, 0.05]
triAssertEq [getDammage caFar, 0]
triAssertEq [getDammage caJeepFar, 0]

// ---- 2. a stomp on a tank ------------------------------------------------------------
caTank = ["T80", 10, -10] call caPut
caJeep = ["UAZ", 10, -30] call caPut
caApc = ["BMP", 30, -22] call caPut
triSimFrames 60
caJeep0 = getPos caJeep
[40, -44, 2.5, 10, -10, 7] call caSee
triSimFrames 6
triScreenshot "a05_tank_before"
if (true) then { [ded, getPos caTank] exec "kj\crush\stomp.sqs"; caT0 = time }
triSimUntil { time > caT0 + KJC_STOMP_WIND + 0.1 }
triScreenshot "a06_tank_blow"
triSimUntil { time > caT0 + KJC_STOMP_WIND + 0.6 }
triScreenshot "a07_tank_ring"
triSimUntil { (not KJ_stomping) and (time > caT0 + 5) }
triScreenshot "a08_tank_after"
logInfo format ["CRUSH stomp tank: result %1 count %2 tank gone %3 jeep dmg %4 moved %5 apc dmg %6 alive %7 far jeep dmg %8 stats %9", KJ_stompResult, KJ_stompCount, isNull caTank, getDammage caJeep, caJeep distance caJeep0, getDammage caApc, alive caApc, getDammage caJeepFar, KJ_crushStats]
triAssertEq [KJ_stompResult, "hit"]
triAssertEq [KJ_stompCount, 2]
triAssert [(isNull caTank) or ((getDammage caTank) >= 1)]
triAssertGe [getDammage caJeep, 1]
triAssert [alive caApc]
triAssertLt [getDammage caApc, 0.5]
triAssertEq [getDammage caJeepFar, 0]
triAssert [alive ded]

// ---- 3. a stomp out of reach lands short -------------------------------------------
if (true) then { [ded, [-100, 0] call caAt] exec "kj\crush\stomp.sqs"; caT0 = time }
triSimUntil { (KJ_stompResult != "wind") and (time > caT0 + KJC_STOMP_WIND + 0.3) }
logInfo format ["CRUSH stomp far: result %1 at %2 m from him", KJ_stompResult, [KJ_stompPos, getPos ded] call KJC_fnDist2]
triAssertEq [KJ_stompResult, "short"]
triAssertLt [[KJ_stompPos, getPos ded] call KJC_fnDist2, KJC_STOMP_REACH * KJC_H + 1]
triSimUntil { not KJ_stomping }

// ---- 4. the kick ---------------------------------------------------------------------
caTank2 = ["T80", 9, 11] call caPut
triSimFrames 60
caTank20 = getPos caTank2
caReach = [ded, caTank2, 1] call KJC_fnReach
logInfo format ["CRUSH kick reach %1", caReach]
triAssertEq [caReach select 0, "kick"]
[44, -26, 2.5, 8, 10, 6] call caSee
triSimFrames 6
triScreenshot "a09_kick_before"
if (true) then { [ded, caTank2] exec "kj\crush\swat.sqs"; caT0 = time }
triSimUntil { time > caT0 + KJC_SWAT_WIND + 0.4 }
triScreenshot "a10_kick_sliding"
triSimUntil { (not KJ_swatting) and (time > caT0 + 5) }
triScreenshot "a11_kick_after"
logInfo format ["CRUSH kick: result %1 how %2 tank moved %3 dmg %4 pos %5", KJ_swatResult, KJ_swatHow, caTank2 distance caTank20, getDammage caTank2, getPos caTank2]
triAssertEq [KJ_swatResult, "hit"]
triAssertEq [KJ_swatHow, "kick"]
triAssertGe [getDammage caTank2, KJC_KICK_DAMAGE]
triAssertGt [caTank2 distance caTank20, 5]

// ---- 5. the swat ---------------------------------------------------------------------
// A crewed Mi-17 with its engine running is carried by the test in a straight
// line past his right shoulder, 26 m out and 36 m up, at 14 m/s. The test plays
// the brain: it orders the swat the moment the helicopter is within reach, and
// lets go of the helicopter when the blow lands.
caPilot = objNull
"Civilian" createUnit [[5230, 3990, 0], caG2, "caPilot = this", 0.5, "PRIVATE"]
caHeli = "Mi17" createVehicle [5230, 4010, 0]
triSimFrames 10
if (true) then { caPilot moveInDriver caHeli; caHeli engineOn true; KJ_swatResult = ""; caHeld = true; caSwung = false; caReach = []; caHd = (getDir ded) + 180; caT0 = time }
[52, -22, 3, 0, 22, 32] call caSee
caCarry = { private ["_p"]; _p = [70 - 14 * (time - caT0), 26] call caAt; caHeli setPos [_p select 0, _p select 1, 36]; caHeli setDir caHd; caHeli setVelocity [14 * sin caHd, 14 * cos caHd, 0] }
triSimUntil { if ((KJ_swatResult == "hit") or (KJ_swatResult == "miss")) then { caHeld = false }; if (caHeld) then { call caCarry }; if (!caSwung) then { if ((([ded, caHeli, 1] call KJC_fnReach) select 0) == "arm") then { caSwung = true; caReach = [ded, caHeli, 1] call KJC_fnReach; [ded, caHeli] exec "kj\crush\swat.sqs" } }; if ((KJ_swatResult == "hit") or (KJ_swatResult == "miss")) then { caHeld = false }; (not caHeld) or (time > caT0 + 12) }
caStruck = getPos caHeli
triScreenshot "a12_swat_blow"
logInfo format ["CRUSH swat: swung %1 reach %2 result %3 how %4 heli %5 vel %6 dmg %7", caSwung, caReach, KJ_swatResult, KJ_swatHow, caStruck, velocity caHeli, getDammage caHeli]
triAssert [caSwung]
triAssertEq [caReach select 0, "arm"]
triAssertEq [KJ_swatResult, "hit"]
triAssertEq [KJ_swatHow, "arm"]
triAssertGe [getDammage caHeli, 1]
if (true) then { caT0 = time }
triSimUntil { time > caT0 + 0.7 }
triScreenshot "a13_swat_falling"
triSimUntil { (((getPos caHeli) select 2) < 6) or (time > caT0 + 8) }
triSimFrames 30
triScreenshot "a14_swat_wreck"
logInfo format ["CRUSH swat end: heli %1 fell %2 s thrown %3 m", getPos caHeli, time - caT0, [getPos caHeli, caStruck] call KJC_fnDist2]
triAssertLt [(getPos caHeli) select 2, 6]
triAssertGt [[getPos caHeli, caStruck] call KJC_fnDist2, 15]
// out of reach: nothing is done
triSimUntil { not KJ_swatting }
if (true) then { [ded, caJeepFar] exec "kj\crush\swat.sqs" }
triSimFrames 30
triAssertEq [KJ_swatResult, "out"]
triAssertEq [getDammage caJeepFar, 0]

// ---- 6. being shot (scripted damage only) --------------------------------------------
caH0 = KJ_hurtCount
if (true) then { ded setDamage 0.3; caT0 = time }
triSimUntil { time > caT0 + 0.6 }
logInfo format ["CRUSH hurt 0.3: dmg %1 hurt %2 count %3 alive %4", getDammage ded, KJ_hurt, KJ_hurtCount, alive ded]
triAssertLt [getDammage ded, 0.01]
triAssertEq [KJ_hurtCount, caH0 + 1]
triAssertEq [KJ_hurt select 3, 1]
triAssertEq [KJ_hurt select 1, -1]
// a heavy shot heard just before the hit gives it a direction: the far jeep "fires"
caBear = (((getPos caJeepFar) select 0) - ((getPos ded) select 0)) atan2 (((getPos caJeepFar) select 1) - ((getPos ded) select 1))
if (caBear < 0) then { caBear = caBear + 360 }
if (true) then { [caJeepFar, "w", "m", "mode", "Shell125"] call KJC_ehFired; ded setDamage 0.004; caT0 = time }
triSimUntil { time > caT0 + 0.6 }
logInfo format ["CRUSH hurt shot: hurt %1 count %2 expected bearing %3 his dir %4", KJ_hurt, KJ_hurtCount, caBear, getDir ded]
triAssertEq [KJ_hurtCount, caH0 + 2]
triAssert [(KJ_hurt select 4) == caJeepFar]
triAssertLt [abs ((KJ_hurt select 1) - caBear), 2]
triAssertGt [KJ_hurt select 3, 0.3]
triAssertLt [KJ_hurt select 3, 0.5]
// a rifle round cannot mark him, so it is not remembered
if (true) then { KJC_lastShot = [-100, objNull, ""]; [caFar, "w", "m", "mode", "Bullet7_6"] call KJC_ehFired }
triAssert [isNull (KJC_lastShot select 1)]
// hurt.sqs has wired the shooters around him by now
triSimUntil { caJeepFar getVariable ["kjcFired", false] }
// a killing blow is undone
if (true) then { ded setDamage 1; caT0 = time }
triSimUntil { (alive ded) and (time > caT0 + 0.6) }
logInfo format ["CRUSH hurt kill: alive %1 dmg %2 revived %3 move %4", alive ded, getDammage ded, KJ_revived, getMove ded]
triAssertEq [KJ_revived, 1]
// forty blows of 0.2 in four seconds do not add up
caI = 0
triSimUntil { if (time > caT0 + 1 + 0.1 * caI) then { ded setDamage ((getDammage ded) + 0.2); caI = caI + 1 }; caI >= 40 }
triSimFrames 30
logInfo format ["CRUSH hurt forty: alive %1 dmg %2 count %3 revived %4", alive ded, getDammage ded, KJ_hurtCount, KJ_revived]
triAssert [alive ded]
triAssertEq [KJ_revived, 1]
// and he still walks
if (true) then { ded stop false; ded doMove ([60, 0] call caAt); caP0 = getPos ded; caT0 = time }
triSimUntil { ((ded distance caP0) > 8) or (time > caT0 + 25) }
logInfo format ["CRUSH hurt walk: moved %1 in %2 s move %3", ded distance caP0, time - caT0, getMove ded]
triAssertGt [ded distance caP0, 8]
triEndTest
