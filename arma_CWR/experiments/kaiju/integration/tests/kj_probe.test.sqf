// ============================================================================
//  Dedushka integration: the probe. Measurements on the kaiju binary that the
//  integrated set is built on. Every KJP line in game_stdout.log is a result.
//  Run: integration\run-kj.ps1 -Name kj_probe
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4900, 4080, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.25
0 setFog 0
setViewDistance 2000
showCinemaBorder false
call compile preprocessFile "kjd_params.sqf"
if (isNil "KJD_class") then { KJD_class = "UDDedushka" }
kjpC = createCenter civilian
kjpG = createGroup civilian
kaiju = objNull
kjpAt = [4900, 4004, 0]
KJD_class createUnit [kjpAt, kjpG, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
kjpNative = (format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_])
triAssert [kjpNative]
// ---- 1. the config block: read before any script has touched him
logInfo format ["KJP config traits: farDraw %1 farContrast %2 wadeDepth %3 obstacleBlind %4 noSpeedDamage %5 damageCeiling %6 hitReport %7 stepShake %8 gridRadius %9", kjGet [kaiju, "farDraw"], kjGet [kaiju, "farContrast"], kjGet [kaiju, "wadeDepth"], kjGet [kaiju, "obstacleBlind"], kjGet [kaiju, "noSpeedDamage"], kjGet [kaiju, "damageCeiling"], kjGet [kaiju, "hitReport"], kjGet [kaiju, "stepShake"], kjGet [kaiju, "gridRadius"]]
logInfo format ["KJP config traits: nearMip %1; class entries kjHeight %2 kjVoice %3 kjFurnacePoint %4", kjGet [kaiju, "nearMip"], KJD_class GetVehicleParam "kjHeight", KJD_class GetVehicleParam "kjVoice", KJD_class GetVehicleParam "kjFurnacePoint"]
if (true) then { kaiju setDir 90; kaiju setPos kjpAt; kaiju setCaptive true; KJ_brainAuto = false; KJ_ready = false; KJT_on = true; [kaiju, "idle"] exec "kj\start.sqs" }
triSimUntil { KJ_ready }
triSimFrames 90
[kaiju] exec "kj\brain\watch.sqs"
logInfo format ["KJP ready: state %1, traits from config %2 of %3, height %4, furnace %5, foot points %6, light count %7, steps %8", KJ_state, KJ_traitsFromConfig, count KJ_traits, KJ_heightM, KJ_furnaceOfs, KJ_hasFootPoints, kjLightCount kaiju, kjSteps kaiju]
logInfo format ["KJP points (his frame, right forward up): furnace %1 stopaL %2 stopaP %3 pruka %4 hlava %5", [kaiju, kjPoint [kaiju, KJ_furnacePoint]] call KJ_fnToLocal, [kaiju, kjPoint [kaiju, "stopaL"]] call KJ_fnToLocal, [kaiju, kjPoint [kaiju, "stopaP"]] call KJ_fnToLocal, [kaiju, kjPoint [kaiju, "pruka"]] call KJ_fnToLocal, [kaiju, kjPoint [kaiju, "hlava"]] call KJ_fnToLocal]
KJD_probe = "HeliHEmpty" createVehicle [4900, 4100, 0]
kjpView = { private ["_g"]; KJD_probe setPos [_this select 0, _this select 1, 0]; _g = ((getPosASL KJD_probe) select 2) + (_this select 2); [_this select 0, _g, _this select 1, ((getPosASL kaiju) select 0) - (_this select 0), ((getPosASL kaiju) select 2) + (_this select 3) - _g, ((getPosASL kaiju) select 1) - (_this select 1)] }
triSetView ([4990, 3975, 1.7, 22] call kjpView)
triSimFrames 6
triScreenshot "00_warmup_discard"

// ---- 3. the start of a walk: the standing foot, with the engine's planted blend off and on
// ---- ([finishing] the trait plantBlend, engine version 2; the two script fixes of the integration are gone from this probe: both made it worse)
kjpV2 = KJ_engineVersion >= 2
if (kjpV2) then { kjSet [kaiju, "plantBlend", 0] }
if (true) then { KJB_START_FIX = false; KJB_strideLoss = 0.6; [kaiju, KJB_plantPt, 2.1, "planted blend off"] exec "lab_foot.sqs"; KJ_brainOrder = ["drawn", [5100, 4004, 0]] }
triSimUntil { !KJL_footBusy }
logInfo format ["KJP start, planted blend off: %1", KJL_foot]
kjpNoFix = KJL_foot
KJ_brainOrder = ["idle"]
triSimUntil { (KJ_state == "idle") && (KJB_mv == "stand") }
triSimFrames 120
if (kjpV2) then { kjSet [kaiju, "plantBlend", 1] }
if (true) then { KJB_strideLoss = 0; [kaiju, KJB_plantPt, 2.1, "planted blend on"] exec "lab_foot.sqs"; KJ_brainOrder = ["drawn", [5100, 4004, 0]] }
triSimUntil { !KJL_footBusy }
logInfo format ["KJP start, planted blend on (engine version %2): %1", KJL_foot, KJ_engineVersion]
kjpFix = KJL_foot
triSetView ([((getPos kaiju) select 0) + 20, 3960, 1.7, 6] call kjpView)
triSimFrames 4
triScreenshot "s01_walking_feet_planted_blend"
KJ_brainOrder = ["idle"]
triSimUntil { (KJ_state == "idle") && (KJB_mv == "stand") }
triSimFrames 120
// a second start with it on, the other foot followed (the foot that swings: it must move forward, not back)
if (true) then { [kaiju, "stopaP", 2.1, "planted blend on, the other foot"] exec "lab_foot.sqs"; KJ_brainOrder = ["drawn", [5100, 4004, 0]] }
triSimUntil { !KJL_footBusy }
logInfo format ["KJP start, planted blend on, the other foot: %1", KJL_foot]
if (true) then { kjpT = time; kjpP = getPos kaiju }
// the walk itself: pace over five seconds, footfalls the engine reports
triSimUntil { time > kjpT + 5 }
logInfo format ["KJP walk: %1 m in %2 s, native steps %3, KJ_footfall %4, crush stats %5", [kjpP, getPos kaiju] call KJB_fnDist, time - kjpT, KJ_stepsNative, KJ_footfall, KJ_crushStats]
KJ_brainOrder = ["idle"]
triSimUntil { (KJ_state == "idle") && (KJB_mv == "stand") }
triSimFrames 60
logInfo format ["KJP after three legs: pos %1 dir %2 track %3 top speed %4 jumps %5", getPos kaiju, getDir kaiju, KJ_brainTrack, KJT_maxSpeed, KJT_jumps]

// ---- 4. the furnace point in the pose: idle, then straightened for the beam
logInfo format ["KJP furnace point idle: %1", [kaiju, kjPoint [kaiju, KJ_furnacePoint]] call KJ_fnToLocal]
kaiju playMove "DedBeamRise"
if (true) then { kjpT = time }
triSimUntil { time > kjpT + 8 }
logInfo format ["KJP furnace point tall (move %1): %2", getMove kaiju, [kaiju, kjPoint [kaiju, KJ_furnacePoint]] call KJ_fnToLocal]
kaiju playMove "DedBeamLower"
if (true) then { kjpT = time }
triSimUntil { time > kjpT + 9 }
logInfo format ["KJP furnace point after lowering (move %1): %2", getMove kaiju, [kaiju, kjPoint [kaiju, KJ_furnacePoint]] call KJ_fnToLocal]

// ---- 5. the direct steer: its pace in the "walk" and the "slow" gait, at a rate, and under the pace trait
// ---- ([finishing] engine version 2: every gait plays its clip at its own speed unless a rate is given)
if (true) then { kjpT = time; kjpP = getPos kaiju; kjSteer [kaiju, 90, "walk"] }
triSimUntil { time > kjpT + 9 }
logInfo format ["KJP steer walk: %1 m in %2 s (move %3), state %4", [kjpP, getPos kaiju] call KJB_fnDist, time - kjpT, getMove kaiju, kjSteerState kaiju]
if (true) then { kjpT = time; kjpP = getPos kaiju; kjSteer [kaiju, 90, "slow"] }
triSimUntil { time > kjpT + 9 }
logInfo format ["KJP steer slow: %1 m in %2 s (move %3), state %4", [kjpP, getPos kaiju] call KJB_fnDist, time - kjpT, getMove kaiju, kjSteerState kaiju]
if (true) then { kjpT = time; kjpP = getPos kaiju; kjSteer [kaiju, 90, "walk", 0, 0.7] }
triSimUntil { time > kjpT + 9 }
logInfo format ["KJP steer walk at rate 0.7: %1 m in %2 s (move %3)", [kjpP, getPos kaiju] call KJB_fnDist, time - kjpT, getMove kaiju]
if (true) then { kjpT = time; kjpP = getPos kaiju; kjSteer [kaiju, 90, "walk"]; kjSet [kaiju, "pace", 0.5] }
triSimUntil { time > kjpT + 9 }
logInfo format ["KJP steer walk under pace 0.5: %1 m in %2 s (move %3), pace reads %4", [kjpP, getPos kaiju] call KJB_fnDist, time - kjpT, getMove kaiju, kjGet [kaiju, "pace"]]
kjSet [kaiju, "pace", 1]
kjSteerStop kaiju
if (true) then { kjpT = time }
triSimUntil { time > kjpT + 4 }

// ---- 6. a thrown man (kjImpulse on a man is in the API but was only run on a tank)
kjpMan = objNull
"Civilian" createUnit [[((getPos kaiju) select 0) + 40, 4050, 0], kjpG, "kjpMan = this", 0.5, "PRIVATE"]
triSimFrames 30
if (true) then { kjpP = getPos kjpMan; kjpMan disableAI "MOVE"; [kjpMan, [(kjpP select 0) - 3, (kjpP select 1)], 1, false] call KJC_fnThrowMan; kjpT = time }
triSimUntil { time > kjpT + 3 }
logInfo format ["KJP thrown man: from %1 to %2 (%3 m), alive %4, damage %5", kjpP, getPos kjpMan, [kjpP, getPos kjpMan] call KJB_fnDist, alive kjpMan, getDammage kjpMan]
deleteVehicle kjpMan

// ---- 7. the sea bed on the demo's line into the bay (x = 5000): he is put under the bed and the engine lifts him onto it
if (true) then { KJT_skip = true; kaiju setPosASL [5000, 3810, -40] }
triSimFrames 20
logInfo format ["KJP bed z 3810: feet asl %1, depth %2", (getPosASL kaiju) select 2, call KJB_fnDepth]
kaiju setPosASL [5000, 3790, -40]
triSimFrames 20
logInfo format ["KJP bed z 3790: feet asl %1, depth %2", (getPosASL kaiju) select 2, call KJB_fnDepth]
kaiju setPosASL [5000, 3770, -40]
triSimFrames 20
logInfo format ["KJP bed z 3770: feet asl %1, depth %2", (getPosASL kaiju) select 2, call KJB_fnDepth]
kaiju setPosASL [5000, 3750, -40]
triSimFrames 20
logInfo format ["KJP bed z 3750: feet asl %1, depth %2", (getPosASL kaiju) select 2, call KJB_fnDepth]
kaiju setPosASL [5000, 3730, -40]
triSimFrames 20
logInfo format ["KJP bed z 3730: feet asl %1, depth %2", (getPosASL kaiju) select 2, call KJB_fnDepth]
// [finishing] the engine's own reading of the same spot, and of dry land (kjGround, version 2: [ground above the sea, metres of water])
if (KJ_engineVersion >= 2) then { logInfo format ["KJP kjGround at [5000, 3730]: %1 (his feet stand at %2 asl in %3 m of water); at [5000, 3790]: %4; on land at [4900, 4004]: %5; malformed: %6", kjGround [5000, 3730], (getPosASL kaiju) select 2, call KJB_fnDepth, kjGround [5000, 3790], kjGround [4900, 4004], kjGround [5000]] }
kaiju setPosASL [5000, 3710, -40]
triSimFrames 20
logInfo format ["KJP bed z 3710: feet asl %1, depth %2", (getPosASL kaiju) select 2, call KJB_fnDepth]
kaiju setPosASL [5000, 3690, -40]
triSimFrames 20
logInfo format ["KJP bed z 3690: feet asl %1, depth %2", (getPosASL kaiju) select 2, call KJB_fnDepth]
kaiju setPosASL [5000, 3660, -40]
triSimFrames 20
logInfo format ["KJP bed z 3660: feet asl %1, depth %2, damage %3", (getPosASL kaiju) select 2, call KJB_fnDepth, getDammage kaiju]
// wading under the brain: three strides north out of the water, from 3730
if (true) then { kaiju setPosASL [5000, 3730, -40]; kaiju setDir 0 }
triSimFrames 40
if (true) then { KJT_skip = false; kjpT = time; kjpP = getPos kaiju; KJ_brainOrder = ["drawn", [5000, 3790, 0]] }
triSimUntil { ((KJ_state == "idle") && (time > kjpT + 3)) || (time > kjpT + 60) }
logInfo format ["KJP wade walk: from %1 to %2 in %3 s, state %4, wade %5, damage %6, alive %7", kjpP, getPosASL kaiju, time - kjpT, KJ_state, KJB_wade, getDammage kaiju, alive kaiju]
triSetView ([5075, 3792, 1.7, 14] call kjpView)
triSimFrames 4
triScreenshot "w01_stands_in_the_shallows"
logInfo format ["KJP end: jumps %1, pushes %2, hurt count %3, steps %4", KJT_jumps, KJ_crushPushes, KJ_hurtCount, KJ_stepsNative]
// the engine's light and emitter must be gone before the game exits (a kjLight alive at exit crashes the exit: integration NOTES, engine request 1)
if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
logInfo format ["KJP RESULT: 0 checks failed: [] (the probe measures; alive %1)", alive kaiju]
triEndTest
