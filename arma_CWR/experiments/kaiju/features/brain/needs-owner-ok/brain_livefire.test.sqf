// ============================================================================
//  NEEDS-OWNER-OK. WRITTEN, NEVER RUN. In this test the garrison of the demo
//  mission fires live weapons at Dedushka, which the standing rule forbids
//  without the owner's yes. Do not run it from a sweep or to "see if it works".
//
//  Dedushka BRAIN under real fire, deciding for himself. The demo mission's
//  garrison (two T-80s and a BMP parked together, two rifle squads) stays as
//  placed; the mission's own quiet creature is taken out and a hostile one is
//  made in its place: a Resistance unit, not captive, dormant west of the town.
//
//  What it would settle (the reading of the source is in
//  kaiju-notes\features\brain\ai-engagement-survey.md):
//   - who opens fire on him and with what (expected with armor = 1000: tank
//     HEAT and the BMP's 73 mm and AT-3 only; not one rifle round)
//   - that the first hard hit wakes him, that he turns to it and is drawn to it
//   - that three armoured vehicles firing from one place earn the beam within
//     about a minute, and that he is spent afterwards
//   - how much damage a HEAT round really does, and that the heal keeps him alive
//   - whether anybody flees or even goes to combat behaviour (expected: nobody)
//  His config side is still EAST in the pipeline's addon, so the garrison will
//  read him as friendly until it is within about 180 m (survey, section 5);
//  the test reveals him and then waits. If nobody fires, that is the finding,
//  and the config proposal in the brain NOTES.md (side = 2) is the fix.
//
//  To run it, after the owner's yes: copy this file and its toml up one
//  folder, then  features\brain\run-brain.ps1 -Name brain_livefire -Mission DedDemo.Eden
// ============================================================================
triSimUntil { alive player }
player allowDammage false
triSimUntil { !(isNil "KJB_ready") }
triSimUntil { !(isNull kaiju) }
// the mission's quiet creature and its timeline go
if (true) then { KJD_plan = []; KJ_brainOn = false }
triSimFrames 30
deleteVehicle kaiju
triSimFrames 10
// the hostile one. A Resistance centre made by script has an empty friend table until told.
kjCtr = createCenter resistance
resistance setFriend [east, 0]
east setFriend [resistance, 0]
kjG = createGroup resistance
ded = objNull
"UDDedushka" createUnit [[4790, 4004, 0], kjG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
if (true) then { ded setDir 90; ded setPos [4790, 4004, 0] }
// every shot the garrison fires is written down: [time, shooter class, weapon, ammo]
kjFired = []
kjNote = { kjFired = kjFired + [[time, typeOf (_this select 0), _this select 1, _this select 4]] }
kjGarrison = [kjdA1, kjdA2, kjdA3, kjdA4, kjdA5, kjdA6, kjdB1, kjdB2, kjdB3, kjdB4, kjdTank, kjdTank2, kjdBmp]
if (true) then { { _x addEventHandler ["Fired", { _this call kjNote }] } forEach kjGarrison }
// the brain decides for himself and starts dormant
if (true) then { KJ_brainAuto = true; KJ_brainOrder = []; KJB_SPENT = 20; KJT_on = true; [ded] exec "kj\brain\start.sqs" }
triSimUntil { KJ_brainOn }
[ded] exec "kj\brain\watch.sqs"
if (true) then { kjT0 = time; { (group _x) reveal ded } forEach [kjdA1, kjdB1, kjdTank, kjdTank2, kjdBmp] }
// 180 s of it, one line every 10 s
kjTick = 0
triSimUntil { if (time > kjT0 + kjTick) then { kjTick = kjTick + 10; logInfo format ["KJLIVE t %1 state %2 phase %3 shots %4 hits %5 damage %6 knows %7 behaviour %8 interest %9", time - kjT0, KJ_state, KJ_brainPhase, count kjFired, KJ_hurtCount, getDammage ded, kjdTank knowsAbout ded, behaviour kjdA1, KJ_interest] }; time > kjT0 + 180 }
logInfo format ["KJLIVE states %1", KJT_states]
logInfo format ["KJLIVE shots %1", kjFired]
logInfo format ["KJLIVE garrison alive %1, squad A fleeing %2, behaviour %3", { alive _x } count kjGarrison, fleeing kjdA1, behaviour kjdA1]
triAssert [alive ded]
triAssert [(count kjFired) > 0]
triAssert ["flinch" in KJT_states]
triAssert ["drawn" in KJT_states]
triAssert ["beam" in KJT_states]
triAssert [({ (_x select 1) in ["SoldierEB", "OfficerE"] } count kjFired) == 0]
if (true) then { KJT_on = false; KJ_brainOn = false }
triEndTest
