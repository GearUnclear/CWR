// ============================================================================
//  Dedushka integration: THE FIGHT (WEAPONS ARE FIRED; allowed by the owner on
//  2026-11-12). The demo mission on the take "short,kill": the beats up to the
//  spent beat, then the kill beat, which is a real fight (kjd_fight.sqs): he
//  is made the garrison's enemy, his brain is given its head, and two tank
//  platoons and what is left of the garrison's armour fire on him by their own
//  choice. Needs the combat feature in the script set (kj\combat).
//
//      run-kj.ps1 -Name kj_fight -Params short,kill
//
//  What is checked: the force fires real rounds and they wound him; he fights
//  back by his own choice (the beam on what hurts him, a blow on the tank that
//  drives under his foot); badly hurt he turns for the sea; he still dies, by
//  the guns and not by the director; the death plays to its end; everything of
//  the set stops; the body stays. Soft checks; the verdict is the KJF RESULT
//  line. (The staged ending, no weapon fired: kj_kill with the take "staged".)
// ============================================================================
triSimUntil { alive player }
player allowDammage false
triSimUntil { !(isNil "KJ_ready") }
triSimUntil { KJ_ready }
kjFails = []
kjChk = { if (!(_this select 0)) then { kjFails = kjFails + [_this select 1]; logInfo format ["KJF CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJF ok: %1", _this select 1] } }
if (true) then { KJD_spentHold = 6; KJB_SPENT = 30; KJ_beamOverride = { KJ_beamT_spent = 30 }; KJT_on = true; [kaiju] exec "kj\brain\watch.sqs"; call compile preprocessFile "kjd_cams.sqf"; KJD_probe = "HeliHEmpty" createVehicle [5100, 4100, 0] }
kjCamObj = "camera" camCreate [5000, 3980, 3]
kjCamObj cameraEffect ["internal", "back"]
showCinemaBorder false
enableRadio false
kjView = { kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov (_this call KJD_fnLens); kjCamObj camCommit 0; [_this, kaiju] call KJD_fnView }
// [degrees clockwise from his front, metres from him, eye height, aim height above his feet, lens] call kjNear : a view from beside him, wherever he is and whichever way he faces
kjNear = { private ["_p", "_cx", "_cy", "_e", "_b"]; _p = getPosASL kaiju; _b = (getDir kaiju) + (_this select 0); _cx = (_p select 0) + (_this select 1) * (sin _b); _cy = (_p select 1) + (_this select 1) * (cos _b); KJD_probe setPos [_cx, _cy, 0]; _e = ((getPosASL KJD_probe) select 2) + (_this select 2); kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov (_this select 4); kjCamObj camCommit 0; [_cx, _e, _cy, (_p select 0) - _cx, (_p select 2) + (_this select 3) - _e, (_p select 1) - _cy] }
kjT0 = time
if (isNil "KJ_woundHealth") then { KJ_woundHealth = -1; KJ_woundStage = 0; KJ_woundRetreat = false; KJ_woundLog = []; KJ_deathPhase = ""; KJ_deathLog = []; KJ_deathStats = []; KJ_combatStats = [0, 0, 0, 0, 0, 0, 0]; KJ_combatByAmmo = [] }
if (isNil "KJD_killLive") then { KJD_killLive = true }
logInfo format ["KJF start: creature %1, native %2, plan %3, combat feature %4, unkillable %5, health %6, live %7", typeOf kaiju, KJ_native, KJD_plan, KJ_hasCombat, KJ_unkillable, KJ_woundHealth, KJD_killLive]
[KJ_hasCombat && (!KJ_unkillable), "the combat feature is in the set and he can die"] call kjChk
[("kill" in KJD_plan) && KJD_killLive, "the take ends with the kill beat, fought for real"] call kjChk
triSetView ("street" call kjView)
triSimFrames 8
triScreenshot "00_warmup_discard"

// ---- the beats before it ---------------------------------------------------------
triSimUntil { ("spent" in KJD_done) || (time > kjT0 + 460) }
[("beam" in KJT_states) && ("spent" in KJD_done), "the beats up to the spent beat played"] call kjChk
triSimUntil { (KJD_step == "kill") || (time > kjT0 + 480) }
triSimUntil { (!(isNil "KJD_fightOn")) || (time > kjT0 + 500) }
if (isNil "KJD_fightOn") then { KJD_fightOn = false; KJD_force = []; KJD_force2 = []; KJD_fightShots = 0; KJD_fightBeams = 0; KJD_fightStomps = 0; KJD_fightByDirector = false; KJD_fightLog = []; KJD_fightCutOff = -1 }
triSimFrames 60
if (true) then { kjT1 = time; kjHits0 = KJ_combatStats select 2; kjWrecks0 = if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }; kjForce0 = { alive _x } count KJD_force; kjSeen = []; kjEv = ""; kjStates0 = count KJT_states }
logInfo format ["KJF fight begins: his side %1, health %2, stage %3, state %4, auto %5; the force %6 (alive %7), the second platoon %8 with crews %9", side kaiju, KJ_woundHealth, KJ_woundStage, KJ_state, KJ_brainAuto, count KJD_force, kjForce0, count KJD_force2, { (count (crew _x)) >= 3 } count KJD_force2]
[(count KJD_force) >= 8, "fight: the force is two platoons and more (8 vehicles or more)"] call kjChk
[({ (count (crew _x)) >= 3 } count KJD_force2) == (count KJD_force2), "fight: the platoon made by script is crewed"] call kjChk
[(side kaiju) == resistance, "fight: he is the garrison's enemy"] call kjChk
triSimUntil { (KJD_fightShots > 0) || (time > kjT1 + 60) }
if (true) then { kjShotAt = time - kjT1 }
triSimFrames 20
triSetView ("north" call kjView)
triSimFrames 6
triScreenshot "13_the_force_opens_fire"

// ---- three moments of it, whichever come: the beam, a blow, the turn for the sea ----
// (a stomp that came down on a tank is noted when it happens: he may stomp again later and fall short)
kjStompHit = false
kjNoteStomp = { if (!(isNil "KJ_stompResult")) then { if (KJ_stompResult == "hit") then { kjStompHit = true } } }
kjWait = { call kjNoteStomp; kjEv = ""; if ((!("beam" in kjSeen)) && (KJ_state == "beam") && (KJ_beamPhase == "fire")) then { kjEv = "beam" }; if ((!("blow" in kjSeen)) && (KJ_state in ["stomp", "swat"])) then { kjEv = "blow" }; if ((!("retreat" in kjSeen)) && KJ_woundRetreat) then { kjEv = "retreat" }; (kjEv != "") || KJ_dead || (time > kjT1 + 880) }
kjEvView = { if (kjEv == "blow") then { [40, 75, 14, 12, 0.9] call kjNear } else { "high" call kjView } }
kjRetAt = []
kjSea = []
kjEvNote = { kjSeen = kjSeen + [kjEv]; kjTe = time; if (kjEv == "retreat") then { kjRetAt = getPos kaiju; kjSea = KJ_woundRetreatPos }; logInfo format ["KJF moment %1: %2 at %3 s into the fight (health %4, stage %5, state %6, phase %7, force alive %8, rounds %9)", _this, kjEv, time - kjT1, KJ_woundHealth, KJ_woundStage, KJ_state, KJ_brainPhase, { alive _x } count KJD_force, KJD_fightShots] }
triSimUntil { call kjWait }
"13b" call kjEvNote
triSimUntil { (time > kjTe + (if (kjEv == "blow") then { 5.7 } else { if (kjEv == "retreat") then { 18 } else { 2 } })) || KJ_dead }
triSetView (call kjEvView)
triSimFrames 6
triScreenshot "13b_a_moment_of_the_fight"
triSimUntil { call kjWait }
"13c" call kjEvNote
triSimUntil { (time > kjTe + (if (kjEv == "blow") then { 5.7 } else { if (kjEv == "retreat") then { 18 } else { 2 } })) || KJ_dead }
triSetView (call kjEvView)
triSimFrames 6
triScreenshot "13c_a_moment_of_the_fight"
triSimUntil { call kjWait }
"13d" call kjEvNote
triSimUntil { (time > kjTe + (if (kjEv == "blow") then { 5.7 } else { if (kjEv == "retreat") then { 18 } else { 2 } })) || KJ_dead }
triSetView (call kjEvView)
triSimFrames 6
triScreenshot "13d_a_moment_of_the_fight"

// ---- he dies ---------------------------------------------------------------------------
triSimUntil { call kjNoteStomp; KJ_dead || (time > kjT1 + 900) }
if (true) then { kjT2 = time }
logInfo format ["KJF dead %1: %2 s into the fight, by the director %3, death phase %4, state %5, brain on %6, health %7, pos %8", KJ_dead, time - kjT1, KJD_fightByDirector, KJ_deathPhase, KJ_state, KJ_brainOn, KJ_woundHealth, getPos kaiju]
triSimUntil { (KJ_deathPhase == "flare") || (time > kjT2 + 30) }
triSimFrames 90
triSetView ([35, 95, 16, 24, 0.9] call kjNear)
triSimFrames 6
triScreenshot "14_dying_the_furnace_flares"
triSimUntil { (KJ_deathPhase == "fall") || (time > kjT2 + 60) }
triSimFrames 150
triSetView ([35, 110, 16, 16, 0.8] call kjNear)
triSimFrames 6
triScreenshot "14b_he_falls"
triSimUntil { (KJ_deathPhase == "body") || (time > kjT2 + 120) }
if (true) then { kjT3 = time }
// the dust of the fall hangs for a while: wait for it to thin
triSimUntil { time > kjT3 + 25 }
triSetView ([35, 110, 16, 6, 0.8] call kjNear)
triSimFrames 6
triScreenshot "14c_the_body"
triSetView ([20, 70, 55, 3, 0.8] call kjNear)
triSimFrames 6
triScreenshot "14d_the_body_from_above"
// [finishing] the body must be there to see: how deep is the water where he died, and does the trunk of the body break the surface
kjDeadDepth = call KJB_fnDepth
kjBodyTop = -999
if (KJ_native) then { kjBodyF = kjPoint [kaiju, KJ_furnacePoint]; if ((count kjBodyF) == 3) then { kjBodyTop = kjBodyF select 2 } }
logInfo format ["KJF body: he died at %1 in %2 m of water (the surf limit is %3 m), at bay %4; the furnace point of the body is %5 m above the sea; set into the ground %6 m", getPos kaiju, kjDeadDepth, KJB_BAY_WADE * KJ_heightM, KJ_brainBay, kjBodyTop, if (isNil "KJ_bodySink") then { -1 } else { KJ_bodySink }]
triSimUntil { ("kill" in KJD_done) || (time > kjT2 + 150) }
logInfo format ["KJF end: death log %1, death stats %2, alive %3, null %4, state %5, brain on %6, furnace on %7, presence on %8, crush on %9, beam active %10, lights %11", KJ_deathLog, KJ_deathStats, alive kaiju, isNull kaiju, KJ_state, KJ_brainOn, KJ_furnaceOn, KJ_presenceOn, KJ_crushOn, if (isNil "KJ_beamActive") then { false } else { KJ_beamActive }, if (KJ_native) then { kjLightCount kaiju } else { -1 }]
logInfo format ["KJF end: seen %1, first shot %2 s in, rounds fired %3, real hits felt %4, beams %5 (wrecks by the beam %6), blows %7 (last stomp %8), force alive %9 of %10 (the cut-off group came at %12 s), wound log %11", kjSeen, kjShotAt, KJD_fightShots, (KJ_combatStats select 2) - kjHits0, KJD_fightBeams, (if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }) - kjWrecks0, KJD_fightStomps, if (isNil "KJ_stompResult") then { "" } else { KJ_stompResult }, { alive _x } count KJD_force, count KJD_force, KJ_woundLog, KJD_fightCutOff]
logInfo format ["KJF end: done %1, states %2, combat stats %3, by ammo %4, jumps %5, fight log %6", KJD_done, KJT_states, KJ_combatStats, KJ_combatByAmmo, KJT_jumps, KJD_fightLog]
[KJD_fightShots >= 10, "fight: the force fired real rounds (10 or more)"] call kjChk
[((KJ_combatStats select 2) - kjHits0) >= 10, "fight: real hits landed and were felt (10 or more)"] call kjChk
[KJD_fightBeams >= 1, "fight: he answered with the beam, by his own choice"] call kjChk
[({ !(alive _x) } count KJD_force) >= 1, "fight: he cost the force at least one vehicle"] call kjChk
[KJD_fightStomps >= 1, "fight: he struck a blow at a vehicle in his reach, by his own choice"] call kjChk
kjToSea = -1
if (((count kjRetAt) > 0) && ((count kjSea) >= 2)) then { kjToSea = ([kjRetAt, kjSea] call KJB_fnDist) - ([getPos kaiju, kjSea] call KJB_fnDist) }
logInfo format ["KJF retreat: flag raised with him at %1, the sea at %2, he died at %3: %4 m nearer the sea; the brain's retreat goal %5, stood %6; stomp result %7", kjRetAt, kjSea, getPos kaiju, kjToSea, KJB_retreatGoal, KJB_retreatStood, if (isNil "KJ_stompResult") then { "" } else { KJ_stompResult }]
["retreat" in kjSeen, "fight: badly hurt, the retreat to the sea was raised"] call kjChk
[(kjToSea > 30) || KJ_brainBay, format ["fight: he walked for the sea (%1 m nearer it when he died; 30 m or more), or, dying on his feet before he could, he turned at bay (%2)", kjToSea, KJ_brainBay]] call kjChk
call kjNoteStomp
[kjStompHit, format ["fight: a stomp of his came down on a tank (the last one's result: %1)", if (isNil "KJ_stompResult") then { "" } else { KJ_stompResult }]] call kjChk
[KJ_dead, "fight: he died"] call kjChk
[kjDeadDepth <= ((KJB_BAY_WADE + 0.06) * KJ_heightM), format ["death: the body can be seen: he died in %1 m of water, no deeper than the surf (%2 m)", kjDeadDepth, (KJB_BAY_WADE + 0.06) * KJ_heightM]] call kjChk
[!KJD_fightByDirector, "fight: the guns killed him, not the director"] call kjChk
[({ alive _x } count KJD_force) >= 1, "fight: some of the force lived to see it"] call kjChk
[KJ_deathPhase == "body", "death: it played to its end"] call kjChk
[({ (_x select 0) in ["stagger", "flare", "vent", "dark", "fall", "down", "body"] } count KJ_deathLog) >= 6, "death: stagger, flare, vent, dark, fall, down and body all came"] call kjChk
[!(alive kaiju), "death: he is dead"] call kjChk
[!(isNull kaiju), "death: the body stays"] call kjChk
[(KJ_state == "dead") && (!KJ_brainOn), "death: the brain has stopped (no walking corpse)"] call kjChk
[(!KJ_furnaceOn) && (!KJ_presenceOn) && (!KJ_crushOn), "death: the furnace loop, the drone and the crush loop have stopped"] call kjChk
[!(if (isNil "KJ_beamActive") then { false } else { KJ_beamActive }), "death: no beam is left burning"] call kjChk
// (a native command may only stand inside a block the stock binary never runs)
kjLights = -1
if (KJ_native) then { kjLights = kjLightCount kaiju }
[(!KJ_native) || (kjLights == 0), "death: no light is left on him"] call kjChk
["kill" in KJD_done, "the beat ended by itself"] call kjChk
logInfo format ["KJF RESULT: %1 checks failed: %2", count kjFails, kjFails]
// ---- the body by daylight: the next morning, from both sides and from above (stills for the sheet) ----
kjMx = ((getPos kaiju) select 0) + 0.5 * KJ_heightM * (sin (getDir kaiju))
kjMy = ((getPos kaiju) select 1) + 0.5 * KJ_heightM * (cos (getDir kaiju))
kjMz = ((getPosASL kaiju) select 2) + 12
if (KJ_native) then { if (KJ_engineVersion >= 2) then { kjMz = ((kjGround [kjMx, kjMy]) select 0) + 5; if (kjMz < 3) then { kjMz = 3 } } }
// [degrees clockwise from the way he fell, metres from the middle of the body, eye height above the ground or the sea there] call kjBodyView
kjBodyView = { private ["_b", "_cx", "_cy", "_e"]; _b = (getDir kaiju) + (_this select 0); _cx = kjMx + (_this select 1) * (sin _b); _cy = kjMy + (_this select 1) * (cos _b); KJD_probe setPos [_cx, _cy, 0]; _e = ((getPosASL KJD_probe) select 2) + (_this select 2); [_cx, _e, _cy, kjMx - _cx, kjMz - _e, kjMy - _cy] }
skipTime (31 - daytime)
triSimFrames 40
kjCamObj camSetFov 0.8
kjCamObj camCommit 0
triSetView ([90, 85, 20] call kjBodyView)
triSimFrames 6
triScreenshot "15_the_next_morning_the_body_from_its_right"
triSetView ([270, 85, 20] call kjBodyView)
triSimFrames 6
triScreenshot "15b_the_next_morning_the_body_from_its_left"
triSetView ([200, 60, 75] call kjBodyView)
triSimFrames 6
triScreenshot "15c_the_next_morning_the_body_from_above"
logInfo format ["KJF the next morning: the middle of the body [%1, %2], aim height %3 asl, the hour %4", kjMx, kjMy, kjMz, daytime]
// the engine's light and emitter must be gone before the game exits (integration NOTES, engine request 1)
if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false; KJ_combatOn = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
// The verdict is the RESULT line above, read by run-kj.ps1 from the game's log (not a failing triAssert).
triEndTest
