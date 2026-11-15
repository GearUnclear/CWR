// ============================================================================
//  Dedushka integration: LIVE FIRE (weapons are fired; allowed by the owner on
//  2026-11-12). Run with the take "hostile": run-kj.ps1 -Name kj_livefire -Params hostile
//
//  The demo mission's garrison against a hostile creature that decides for
//  himself (the integrated set, brain on auto, dormant west of the town). It
//  settles what no quiet run could:
//   - who opens fire on him and with what (read from source: only ammo with
//     hit of about 168 or more; not one rifle round)
//   - that a real hit raises the hit signal with the real shooter (the engine's
//     hit report on the kaiju binary; the crush feature's shot memory on stock)
//   - that the first hard hit wakes him, that he flinches and is drawn to it
//   - whether massed armour firing from one place earns the beam
//   - how much a real round adds, and that he survives it while unkillable
//  It is mostly a record (KJLIVE lines); the hard checks are few and soft.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
triSimUntil { !(isNil "KJ_ready") }
triSimUntil { KJ_ready }
triAssert [KJD_hostile]
kjFails = []
kjChk = { if (!(_this select 0)) then { kjFails = kjFails + [_this select 1]; logInfo format ["KJLIVE CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJLIVE ok: %1", _this select 1] } }
kjFired = []
kjNote = { if ((count kjFired) < 400) then { kjFired = kjFired + [[time, typeOf (_this select 0), _this select 1, _this select 4]] } }
kjGarrison = [kjdA1, kjdA2, kjdA3, kjdA4, kjdA5, kjdA6, kjdB1, kjdB2, kjdB3, kjdB4, kjdTank, kjdTank2, kjdBmp]
if (true) then { { _x addEventHandler ["Fired", { _this call kjNote }] } forEach kjGarrison; KJB_SPENT = 25; KJ_beamOverride = { KJ_beamT_spent = 25 }; KJT_on = true; [kaiju] exec "kj\brain\watch.sqs"; call compile preprocessFile "kjd_cams.sqf"; KJD_probe = "HeliHEmpty" createVehicle [5100, 4100, 0] }
// a camera object takes the view, so no crosshair or HUD is in the frames (triSetView then places the view)
kjCamObj = "camera" camCreate [5000, 3980, 3]
kjCamObj cameraEffect ["internal", "back"]
showCinemaBorder false
enableRadio false
// a spot's view: its lens on the camera object, its place and aim through triSetView
kjView = { kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov (_this call KJD_fnLens); kjCamObj camCommit 0; [_this, kaiju] call KJD_fnView }
logInfo format ["KJLIVE start: creature %1 side %2, native %3, unkillable %4, combat feature %5, state %6", typeOf kaiju, side kaiju, KJ_native, KJ_unkillable, KJ_hasCombat, KJ_state]
// the garrison is told where he is (his config side reads as friendly at range)
if (true) then { kjT0 = time; { (group _x) reveal kaiju } forEach [kjdA1, kjdB1, kjdTank, kjdTank2, kjdBmp] }
triSetView ("armour" call kjView)
triSimFrames 8
triScreenshot "00_warmup_discard"
kjTick = 0
kjShotAt = -1
// 200 s of it: one line every 10 s; frames at the first shot, the first flinch and the beam
triSimUntil { if (time > kjT0 + kjTick) then { kjTick = kjTick + 10; logInfo format ["KJLIVE t %1 state %2 phase %3 shots %4 hits %5 hurt %6 damage %7 knows %8 behaviour %9 wound %10", time - kjT0, KJ_state, KJ_brainPhase, count kjFired, KJ_hurtCount, KJ_hurt, getDammage kaiju, kjdTank knowsAbout kaiju, behaviour kjdA1, call KJ_fnWound] }; ((count kjFired) > 0) || (time > kjT0 + 200) }
if (true) then { kjShotAt = time - kjT0 }
triSetView ("armour" call kjView)
triSimFrames 30
triScreenshot "L01_the_first_shots"
triSimUntil { if (time > kjT0 + kjTick) then { kjTick = kjTick + 10; logInfo format ["KJLIVE t %1 state %2 phase %3 shots %4 hits %5 hurt %6 damage %7 knows %8 behaviour %9 wound %10", time - kjT0, KJ_state, KJ_brainPhase, count kjFired, KJ_hurtCount, KJ_hurt, getDammage kaiju, kjdTank knowsAbout kaiju, behaviour kjdA1, call KJ_fnWound] }; ("flinch" in KJT_states) || (!(alive kaiju)) || (time > kjT0 + 200) }
triSetView ("square" call kjView)
triSimFrames 60
triScreenshot "L02_hit_he_shudders"
triSimUntil { if (time > kjT0 + kjTick) then { kjTick = kjTick + 10; logInfo format ["KJLIVE t %1 state %2 phase %3 shots %4 hits %5 hurt %6 damage %7 knows %8 behaviour %9 wound %10", time - kjT0, KJ_state, KJ_brainPhase, count kjFired, KJ_hurtCount, KJ_hurt, getDammage kaiju, kjdTank knowsAbout kaiju, behaviour kjdA1, call KJ_fnWound] }; ((!(isNil "KJ_beamPhase")) && (KJ_beamPhase == "fire")) || (!(alive kaiju)) || KJ_dead || (time > kjT0 + 200) }
triSetView ("high" call kjView)
triSimFrames 90
triScreenshot "L03_beam_or_end"
triSimUntil { (time > kjT0 + 200) || ((KJ_state == "spent") && (time > kjT0 + 60)) || KJ_dead }
logInfo format ["KJLIVE end at %1 s: state %2, alive %3, dead %4, damage %5, hurt count %6, revived %7, wound %8", time - kjT0, KJ_state, alive kaiju, KJ_dead, getDammage kaiju, KJ_hurtCount, KJ_revived, call KJ_fnWound]
logInfo format ["KJLIVE states %1", KJT_states]
logInfo format ["KJLIVE first shot %1 s in; shots %2", kjShotAt, count kjFired]
logInfo format ["KJLIVE shots (time, shooter, weapon, ammo): %1", kjFired]
logInfo format ["KJLIVE last hurt %1; last engine hit report %2", KJ_hurt, if (isNil "KJ_hurtReport") then { [] } else { KJ_hurtReport }]
logInfo format ["KJLIVE garrison alive %1 of %2, squad A fleeing %3, behaviour %4; beam shots %5 kills %6 wrecks %7; crush %8", { alive _x } count kjGarrison, count kjGarrison, fleeing kjdA1, behaviour kjdA1, if (isNil "KJ_beamShots") then { 0 } else { KJ_beamShots }, if (isNil "KJ_beamKills") then { 0 } else { KJ_beamKills }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }, KJ_crushStats]
logInfo format ["KJLIVE jumps %1; pushes %2", KJT_jumps, KJ_crushPushes]
[(count kjFired) > 0, "the garrison fired on him"] call kjChk
[({ (_x select 1) in ["SoldierEB", "OfficerE"] } count kjFired) == 0, "no rifleman fired at him (bullets cannot mark him)"] call kjChk
[KJ_hurtCount > 0, "real hits raised the hit signal"] call kjChk
[(KJ_hurtCount == 0) || (!(isNull (KJ_hurt select 4))), "the hit signal names a shooter"] call kjChk
["flinch" in KJT_states, "a hard hit made him flinch"] call kjChk
[("drawn" in KJT_states) || ("beam" in KJT_states), "he was drawn to it, or answered with the beam"] call kjChk
[(!KJ_unkillable) || (alive kaiju), "unkillable: he lives through it"] call kjChk
logInfo format ["KJLIVE RESULT: %1 checks failed: %2", count kjFails, kjFails]
// the engine's light and emitter must be gone before the game exits (a kjLight alive at exit crashes the exit: integration NOTES, engine request 1)
if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
// The verdict is the RESULT line above, read by run-kj.ps1 from the game's log. It is NOT a failing
// triAssert: a game with a live creature that fails an assert stops answering the harness and holds the
// machine's one game slot until the test's time limit (seen here and by the brain feature).
triEndTest
