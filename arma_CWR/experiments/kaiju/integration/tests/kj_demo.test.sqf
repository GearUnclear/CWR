// ============================================================================
//  Dedushka integration: the whole demo (DedushkaDemo.Eden) on its timeline,
//  every beat checked and photographed. The same file runs on the kaiju binary
//  and on the stock one: where the two differ the check asks KJ_native.
//
//  Checks are soft: each failure is logged ("KJI CHECK FAILED") and collected,
//  and the whole list is logged once at the end ("KJI RESULT: n checks
//  failed"). run-kj.ps1 reads that line: the run passed when n is 0. Every wait
//  has its own time limit, so a beat that never comes fails its check and the
//  run goes on to the next one.
//
//  To keep a run short the test shortens the spent time (the mission plays it
//  in full). Everything else is the mission's own.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
triSimUntil { !(isNil "KJ_ready") }
triSimUntil { KJ_ready }
kjFails = []
kjChk = { if (!(_this select 0)) then { kjFails = kjFails + [_this select 1]; logInfo format ["KJI CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJI ok: %1", _this select 1] } }
kjHas = { _this in KJD_plan }
if (true) then { KJD_spentHold = 16; KJB_SPENT = 30; KJ_beamOverride = { KJ_beamT_spent = 30 }; KJT_on = true; [kaiju] exec "kj\brain\watch.sqs"; call compile preprocessFile "kjd_cams.sqf"; KJD_probe = "HeliHEmpty" createVehicle [5100, 4100, 0] }
// a camera object takes the view, so no crosshair or HUD is in the frames (triSetView then places the view)
kjCamObj = "camera" camCreate [5000, 3980, 3]
kjCamObj cameraEffect ["internal", "back"]
showCinemaBorder false
enableRadio false
// a spot's view: its lens on the camera object, its place and aim through triSetView
kjView = { kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov (_this call KJD_fnLens); kjCamObj camCommit 0; [_this, kaiju] call KJD_fnView }
kjT0 = time
kjStart = getPos kaiju
logInfo format ["KJI start: creature %1, native %2, plan %3, hour %4, start %5, traits from config %6 of %7, unkillable %8", typeOf kaiju, KJ_native, KJD_plan, daytime, kjStart, KJ_traitsFromConfig, count KJ_traits, KJ_unkillable]
[(!KJ_native) || (KJ_traitsFromConfig == (count KJ_traits)), "every engine trait came from the addon's class Kaiju block"] call kjChk
triSetView ("street" call kjView)
triSimFrames 8
triScreenshot "00_warmup_discard"

// ---- glow: dormant, seen from far out on the bay -------------------------------
triSimUntil { ((KJ_state == "dormant") && (KJ_brainPhase == "still")) || (time > kjT0 + 40) }
[(KJ_state == "dormant") && (KJ_brainPhase == "still"), "glow: he stands dormant"] call kjChk
triSetView ("farther" call kjView)
triSimFrames 12
kjFar = [0, 0, 0, 0, 0]
if (KJ_native) then { kjFar = kjFarState kaiju }
logInfo format ["KJI glow: far state %1 (mode, metres, fog floor, frames normal, frames far), furnace %2, lights %3", kjFar, KJ_furnaceNow, if (KJ_native) then { kjLightCount kaiju } else { -1 }]
[(!KJ_native) || (((kjFar select 0) == 2) && ((kjFar select 4) > 0)), "glow: drawn by the far pass, past the view distance, from 2.3 km"] call kjChk
[(KJ_furnaceNow select 2) == "dormant", "glow: the furnace is at its dormant ember"] call kjChk
// (a native command may only stand inside a block the stock binary never runs)
kjLights = -1
if (KJ_native) then { kjLights = kjLightCount kaiju }
[(!KJ_native) || (kjLights >= 1), "glow: the furnace light is on him"] call kjChk
triScreenshot "01c_from_past_the_view_distance"
triSetView ("far" call kjView)
triSimFrames 12
kjFar1 = [0, 0, 0, 0, 0]
if (KJ_native) then { kjFar1 = kjFarState kaiju }
logInfo format ["KJI glow: far state from 1.85 km %1", kjFar1]
[(!KJ_native) || ((kjFar1 select 0) >= 1), "glow: drawn past the object distance from 1.85 km"] call kjChk
triScreenshot "01_glow_far_out_on_the_bay"
triSetView ("west" call kjView)
triSimFrames 6
triScreenshot "01b_dormant_from_the_road_in"

// ---- wake -------------------------------------------------------------------------
triSimUntil { ("wake" in KJD_done) || (time > kjT0 + 140) }
[("wake" in KJD_done) && (KJ_state != "dormant"), "wake: he has risen and stands"] call kjChk
[(KJ_furnaceNow select 2) == "awake", "wake: the furnace has come up"] call kjChk
triSetView ("street" call kjView)
triSimFrames 6
triScreenshot "02_risen"
logInfo format ["KJI wake: %1 s in, state %2, moves so far %3", time - kjT0, KJ_state, KJT_moves]

// ---- through the houses -------------------------------------------------------------
triSimUntil { (((getPos kaiju) select 0) > ((kjStart select 0) + 45)) || (time > kjT0 + 220) }
triSetView ("west" call kjView)
triSimFrames 6
triScreenshot "03_sets_off_toward_the_town"
triSimUntil { (((getPos kaiju) select 0) > 4905) || (time > kjT0 + 260) }
triSetView ("street" call kjView)
triSimFrames 6
triScreenshot "03b_walking_in"
triSimUntil { (((getPos kaiju) select 0) > 4962) || (time > kjT0 + 300) }
triSetView ("square" call kjView)
triSimFrames 6
triScreenshot "03c_among_the_houses"
triSimUntil { ("through" in KJD_done) || (time > kjT0 + 420) }
logInfo format ["KJI through: at %1 after %2 s, dir %3, track %4, crush %5, footfalls %6 (engine %7), steps heard %8, jumps %9, top speed %10", getPos kaiju, time - kjT0, getDir kaiju, KJ_brainTrack, KJ_crushStats, KJ_footfall select 0, KJ_stepsNative, KJ_stepCount, KJT_jumps, KJT_maxSpeed]
["through" in KJD_done, "through: he arrived"] call kjChk
[([getPos kaiju, KJD_town] call KJB_fnDist) < 14, "through: he stands where he was sent, in the row"] call kjChk
[(abs (((getPos kaiju) select 1) - (KJD_town select 1))) < 4, "through: he held his line"] call kjChk
[(KJ_crushStats select 0) >= 2, "through: houses came down in his way"] call kjChk
[(KJ_footfall select 0) >= (((KJ_brainTrack select 2) / (0.3 * KJ_heightM)) - 4), "through: a footfall for every step"] call kjChk
[(!KJ_native) || (KJ_stepsNative >= 10), "through: the footfalls were the engine's own events"] call kjChk
[KJ_stepCount >= ((KJ_footfall select 0) - 2), "through: every footfall had its dust, thump and shake"] call kjChk
[(count KJ_shakeLog) >= 5, "through: the view shook"] call kjChk
[(count KJT_jumps) == 0, "through: nothing shoved him"] call kjChk
kjFellThrough = KJ_crushStats select 0

// ---- roar ---------------------------------------------------------------------------
triSimUntil { (KJ_roarCount >= 1) || (time > kjT0 + 460) }
if (true) then { kjT1 = time; kjA1 = kjdA1 distance kaiju }
triSimUntil { time > kjT1 + 4.6 }
triSetView ("square" call kjView)
triSimFrames 6
triScreenshot "04_roar_from_among_the_garrison"
triSimUntil { ("roar" in KJD_done) || (time > kjT1 + 40) }
logInfo format ["KJI roar: count %1, native %2, panic log %3 entries, roar log %4 entries, A1 fleeing %5 (%6 m then, %7 m now), moves %8", KJ_roarCount, KJ_roarNative, count KJ_panicLog, count KJ_roarLog, fleeing kjdA1, kjA1, kjdA1 distance kaiju, KJT_moves]
[KJ_roarCount == 1, "roar: one roar"] call kjChk
["dedroar" in KJT_moves, "roar: the roar clip played"] call kjChk
[(count KJ_panicLog) >= 2, "roar: the garrison's sections broke"] call kjChk
[(kjdA1 distance kaiju) > (kjA1 + 15), "roar: the nearest section is running from him"] call kjChk
[(!KJ_native) || ((KJ_roarNative select 0) >= 3), "roar: the engine told the groups where he is"] call kjChk
[({ (_x select 0) == ("KJ_Roar" call KJ_fnVoice) } count KJ_sndLog) >= 1, "roar: the roar sound was sent"] call kjChk

// ---- the armour fires ------------------------------------------------------------------
triSimUntil { (KJ_state == "flinch") || (time > kjT1 + 70) }
if (true) then { kjT2 = time }
[KJ_state == "flinch", "fire: the hit stopped him (the brain answered the hit signal by itself)"] call kjChk
[KJ_hurtCount >= 1, "fire: the hit signal was raised"] call kjChk
[(KJ_hurt select 4) in [kjdTank, kjdTank2, kjdBmp], "fire: the signal names the shooter"] call kjChk
[(!KJ_native) || ((count KJD_fireLog) >= 1), "fire: scripted damage went through the engine's damage path"] call kjChk
triSimUntil { (KJ_brainPhase == "shudder") || (time > kjT2 + 12) }
triSimFrames 30
triSetView ("armour" call kjView)
triSimFrames 6
triScreenshot "05_the_armour_fires_he_shudders"
triSimUntil { ("fire" in KJD_done) || (time > kjT2 + 90) }
logInfo format ["KJI fire: over %1 s after the hit, facing %2, tank bears %3, hurt %4, count %5, fire log %6, his damage %7, alive %8", time - kjT2, getDir kaiju, [getPos kaiju, getPos kjdTank] call KJB_fnBear, KJ_hurt, KJ_hurtCount, KJD_fireLog, getDammage kaiju, alive kaiju]
["fire" in KJD_done, "fire: the reaction ran to its end"] call kjChk
["dedflinch" in KJT_moves, "fire: the flinch clip played"] call kjChk
[(abs ([getDir kaiju, [getPos kaiju, getPos ((KJD_fireLog select 0) select 1)] call KJB_fnBear] call KJB_fnAng)) < 6, "fire: he turned to where the first hit came from"] call kjChk
[alive kaiju, "fire: he is alive"] call kjChk
[(!KJ_unkillable) || ((getDammage kaiju) < 0.05), "fire: he was healed (unkillable)"] call kjChk

// ---- the beam ----------------------------------------------------------------------------
triSimUntil { ((!(isNil "KJ_beamPhase")) && (KJ_beamActive)) || (time > kjT2 + 130) }
triSimUntil { (KJ_beamPhase == "charge") || (time > kjT2 + 160) }
if (true) then { kjT3 = time; kjDir0 = getDir kaiju; kjPos0 = getPos kaiju }
[KJ_beamPhase == "charge", "beam: it charges"] call kjChk
triSimUntil { time > kjT3 + 5.5 }
triSetView ("north" call kjView)
triSimFrames 6
triScreenshot "06_straightened_the_furnace_charging"
logInfo format ["KJI charge: furnace %1, lights %2, charge %3, move %4", KJ_furnaceNow, if (KJ_native) then { kjLightCount kaiju } else { -1 }, KJ_beamCharge, getMove kaiju]
triSimUntil { (KJ_beamPhase == "fire") || (time > kjT3 + 40) }
if (true) then { kjT4 = time }
[KJ_beamPhase == "fire", "beam: it fires"] call kjChk
[(count KJ_beamSegs) > 0, "beam: the line is drawn"] call kjChk
triSimUntil { time > kjT4 + 4 }
triSetView ("north" call kjView)
triSimFrames 6
triScreenshot "07_the_beam_from_the_north"
triSimUntil { time > kjT4 + 9 }
triSetView ("high" call kjView)
triSimFrames 6
triScreenshot "07b_the_cut_from_above"
triSimUntil { time > kjT4 + 13 }
triSetView ("armour" call kjView)
triSimFrames 6
triScreenshot "07c_the_beam_from_behind_the_armour"
triSimUntil { (KJ_state == "spent") || (time > kjT4 + 80) }
if (true) then { kjT5 = time }
logInfo format ["KJI beam: out %1 s after it fired; shots %2 kills %3 wrecks %4 razed %5 queued %6 fires %7; sounds %8; furnace log %9; turned %10 deg, moved %11 m; tank alive %12", time - kjT4, KJ_beamShots, KJ_beamKills, KJ_beamWrecks, KJ_beamRazed, count KJ_beamQueue, KJ_beamFires, KJ_beamSnd, KJ_furnaceLog, [getDir kaiju, kjDir0] call KJB_fnAng, [getPos kaiju, kjPos0] call KJB_fnDist, alive kjdTank]
[KJ_state == "spent", "beam: he is spent after it"] call kjChk
[KJ_beamShots == 1, "beam: one beam"] call kjChk
[!(alive kjdTank), "beam: the tank it was aimed at is destroyed"] call kjChk
[(KJ_beamRazed + (count KJ_beamQueue)) >= 8, "beam: the town along the cut came down"] call kjChk
[KJ_beamFires >= 4, "beam: fires are left along the cut"] call kjChk
[("dedbeamrise" in KJT_moves) && ("dedbeamlower" in KJT_moves), "beam: he rose for it and lowered after it"] call kjChk
[(abs ([getDir kaiju, kjDir0] call KJB_fnAng)) < 2, "beam: his feet stayed planted while it cut"] call kjChk
[({ (_x select 0) == "KJ_BeamCharge" } count KJ_beamSnd) + ({ (_x select 0) == "KJ_Beam" } count KJ_beamSnd) + ({ (_x select 0) == "KJ_Exhale" } count KJ_beamSnd) == 3, "beam: charge, beam and spent sounds were sent"] call kjChk
[({ (_x select 0) == "KJ_BeamLoop" } count KJ_beamSnd) >= 2, "beam: the beam's loop carried the cut"] call kjChk
[({ (_x select 1) == "charge" } count KJ_furnaceLog) + ({ (_x select 1) == "fire" } count KJ_furnaceLog) >= 2, "beam: the furnace went through charge and fire"] call kjChk
[({ (_x select 3) == "NATIVE" } count KJ_shakeLog) > 0 || (!KJ_native), "beam: the tremor was the engine's camera shake"] call kjChk

// ---- spent among the fires -------------------------------------------------------------------
triSimUntil { time > kjT5 + 7 }
triSetView ("north" call kjView)
triSimFrames 6
triScreenshot "08_spent_among_the_fires"
triSetView ("street" call kjView)
triSimFrames 6
triScreenshot "08c_spent_from_the_street"
triSetView ("high" call kjView)
triSimFrames 6
triScreenshot "08b_spent_the_town_from_above"
[(KJ_furnaceNow select 2) == "spent", "spent: the furnace is down to a dull red"] call kjChk
triSimUntil { ("spent" in KJD_done) || (!("spent" call kjHas)) || (time > kjT5 + 120) }

// ---- out to sea ----------------------------------------------------------------------------------
triSimUntil { (KJD_step == "sea") || (!("sea" call kjHas)) || (time > kjT5 + 130) }
if (true) then { kjT6 = time; kjFell0 = KJ_crushStats select 0 }
triSimUntil { (((getPos kaiju) select 1) < 3935) || (!("sea" call kjHas)) || (time > kjT6 + 120) }
triSetView ("south" call kjView)
triSimFrames 6
triScreenshot "09_turned_south_toward_the_sea"
triSimUntil { (((getPos kaiju) select 1) < 3820) || (!("sea" call kjHas)) || (time > kjT6 + 200) }
triSetView ("shore" call kjView)
triSimFrames 6
triScreenshot "09b_down_to_the_shore"
triSimUntil { ("sea" in KJD_done) || (!("sea" call kjHas)) || (time > kjT6 + 420) }
logInfo format ["KJI sea: at %1 (asl %2) after %3 s, depth %4, wade %5, blocked by water %6 times, damage %7, alive %8, state %9", getPos kaiju, (getPosASL kaiju) select 2, time - kjT6, call KJB_fnDepth, KJB_wade, KJ_brainTrack select 3, getDammage kaiju, alive kaiju, KJ_state]
triSetView ("shore" call kjView)
triSimFrames 6
triScreenshot "10_wading_out"
triSetView ("water" call kjView)
triSimFrames 6
triScreenshot "10b_in_the_bay_from_the_water"
[(!("sea" call kjHas)) || ("sea" in KJD_done), "sea: the walk ended by itself"] call kjChk
[(!("sea" call kjHas)) || (!KJB_wade) || ((call KJB_fnDepth) > 4), "sea: he stands in deep water (wading)"] call kjChk
[(!("sea" call kjHas)) || KJB_wade || ((KJ_brainTrack select 3) >= 1), "sea: without wading he stopped at the water's edge"] call kjChk
[(!("sea" call kjHas)) || (((getPos kaiju) select 1) < 3830), "sea: he went down to the bay"] call kjChk
[alive kaiju, "sea: he is alive"] call kjChk

// ---- dormant in the water, the glow from far out -----------------------------------------------
triSimUntil { (KJD_step == "done") || (time > kjT6 + 480) }
triSetView ("far" call kjView)
triSimFrames 12
triScreenshot "11_dormant_in_the_bay_from_far_out"
logInfo format ["KJI end: step %1 done %2 state %3 phase %4 pos %5 after %6 s, track %7", KJD_step, KJD_done, KJ_state, KJ_brainPhase, getPos kaiju, time - kjT0, KJ_brainTrack]
logInfo format ["KJI end: states %1", KJT_states]
logInfo format ["KJI end: moves %1", KJT_moves]
logInfo format ["KJI end: jumps (time, metres, seconds, from, to, men, vehicles) %1", KJT_jumps]
logInfo format ["KJI end: pushes cleared from his feet %1", KJ_crushPushes]
logInfo format ["KJI end: top speed %1, crush %2, footfalls %3, engine steps %4, start fix %5, furnace log %6", KJT_maxSpeed, KJ_crushStats, KJ_footfall select 0, KJ_stepsNative, KJ_brainStartFix, KJ_furnaceLog]
[KJD_step == "done", "end: the timeline ran through"] call kjChk
[(count KJD_done) == (count KJD_plan), "end: every beat of the plan was played"] call kjChk
[(!("dormant" call kjHas)) || (KJ_state == "dormant"), "end: he is dormant again"] call kjChk
[(count KJT_jumps) == 0, "end: nothing shoved him in the whole run"] call kjChk
[KJT_maxSpeed < 3.4, "end: never faster than his walk"] call kjChk
[alive kaiju, "end: he is alive"] call kjChk
logInfo format ["KJI RESULT: %1 checks failed: %2", count kjFails, kjFails]
// the engine's light and emitter must be gone before the game exits (a kjLight alive at exit crashes the exit: integration NOTES, engine request 1)
if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
// The verdict is the RESULT line above, read by run-kj.ps1 from the game's log. It is NOT a failing
// triAssert: a game with a live creature that fails an assert stops answering the harness and holds the
// machine's one game slot until the test's time limit (seen here and by the brain feature).
triEndTest
