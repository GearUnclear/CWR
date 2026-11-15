// ============================================================================
//  Dedushka CRUSH: walking through the world (experiment, kept out of
//  tests/integration). He walks south in a straight line from the edge of
//  Morton's green (Everon), over the road and through two houses. Quiet:
//  nobody is armed, nobody fires. Needs the crush variant of the creature
//  addon (@kj_crush). Run it through run-crush.ps1.
//
//  The walk is driven with playMove "DedWalk", six cycles of the walk clip:
//  the stock AI plans a path round every house and tree like a pedestrian, so
//  a doMove would not take him through a house (NOTES.md, "Walking").
//
//  On the open ground, placed from where his feet will land (a foot lands
//  every KJC_HALF_STRIDE x height of ground, right foot first):
//    - a T-80 and a jeep in the middle of a foot lane are flattened;
//    - a jeep on the edge of a foot lane is wrecked and kicked aside;
//    - a civilian under a footfall dies; one 8 m beside it is thrown down and
//      hurt but lives; a civilian and a jeep 45 m to the side are untouched;
//    - no hull stops him: he keeps his pace all the way.
//  From the houses on, the test finds its own evidence: on every sample no
//  building that still stands may have its centre inside his legs, every
//  building he felled got its dust at least a beat before its collapse, and at
//  the end buildings and small things have gone down and he is alive.
//
//  Site (from the map's object list): the line x = 5036 runs south from
//  z = 3925 over open ground and the road, past a town sign, then through the
//  house at (5033, 3854), two hornbeams and the house at (5038, 3829). The
//  church stands 21 m east of the start, outside his strip, on purpose: the
//  stock engine never shows a church collapsing (NOTES.md, "Engine limits").
//  He faces south, so "right" below is west and the cameras stand to the east.
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
cwFrom = [5036, 3925, 0]
cwC = createCenter civilian
cwG = createGroup civilian
cwG2 = createGroup civilian
ded = objNull
"UDDedushka" createUnit [cwFrom, cwG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
if (true) then { ded setCaptive true; ded setBehaviour "CARELESS"; ded allowFleeing 0; ded setDir 180; ded setPos cwFrom; ded stop true }
call compile preprocessFile "kj\crush\init.sqf"
[ded] call KJC_fnSize
logInfo format ["CRUSH walk: height %1 step file %2", KJC_H, KJC_stepFile]
cwCam = "camera" camCreate [5060, 3700, 30]
cwCam cameraEffect ["internal", "back"]
if (true) then { cwCam camSetTarget ded; cwCam camSetFov 0.7; cwCam camCommit 0 }
triSimFrames 30
// cwAt: [forward, right] in his frame -> world position.
// cwSee: camera at [forward, right, up] in his frame, looking at [forward, right, up].
cwAt = { [ded, _this select 0, _this select 1] call KJC_fnWorld }
cwSee = { private ["_a", "_b", "_g"]; _a = [ded, _this select 0, _this select 1] call KJC_fnWorld; _b = [ded, _this select 3, _this select 4] call KJC_fnWorld; _g = (getPosASL ded) select 2; triSetView [_a select 0, _g + (_this select 2), _a select 1, (_b select 0) - (_a select 0), (_this select 5) - (_this select 2), (_b select 1) - (_a select 1)] }
// createVehicle will not put a vehicle near him (it looks for free room around his whole radius), so each is made far off and then set in place.
cwPut = { private ["_v"]; _v = (_this select 0) createVehicle [5230, 4000, 0]; _v setPos ([_this select 1, _this select 2] call cwAt); _v setDir 0; _v }
// Setting off from a stand, footfall k lands (k x half stride - start lag + foot ahead) in front of the start, right for odd k.
cwHs = KJC_HALF_STRIDE * KJC_H
cwFa = (KJC_FOOT_AHEAD - KJC_START_LAG) * KJC_H
cwSt = KJC_STANCE * KJC_H
cwTank = ["T80", 2 * cwHs + cwFa, 0 - cwSt] call cwPut
cwJeep = ["UAZ", 3 * cwHs + cwFa, cwSt] call cwPut
cwJeepEdge = ["UAZ", 3.7 * cwHs + cwFa, cwSt + 3.5] call cwPut
cwJeepFar = ["UAZ", 2 * cwHs, -45] call cwPut
cwUnder = objNull
cwBeside = objNull
cwFar = objNull
"Civilian" createUnit [[4 * cwHs + cwFa, 0 - cwSt] call cwAt, cwG2, "cwUnder = this", 0.5, "PRIVATE"]
"Civilian2" createUnit [[4 * cwHs + cwFa, 0 - cwSt - 8] call cwAt, cwG2, "cwBeside = this", 0.5, "PRIVATE"]
"Civilian3" createUnit [[4 * cwHs, 45] call cwAt, cwG2, "cwFar = this", 0.5, "PRIVATE"]
triSimFrames 60
// createUnit puts a man in his group's formation, not on the spot asked for: set each in place
if (true) then { cwUnder setPos ([4 * cwHs + cwFa, 0 - cwSt] call cwAt); cwBeside setPos ([4 * cwHs + cwFa, 0 - cwSt - 8] call cwAt); cwFar setPos ([4 * cwHs, 45] call cwAt) }
{ _x stop true } forEach [cwUnder, cwBeside, cwFar]
triSimFrames 10
cwJeepEdge0 = getPos cwJeepEdge
logInfo format ["CRUSH walk staged: tank %1 jeep %2 edge jeep %3 under %4 beside %5", getPos cwTank, getPos cwJeep, cwJeepEdge0, getPos cwUnder, getPos cwBeside]
[34, -44, 3, 22, 0, 8] call cwSee
triSimFrames 6
triScreenshot "w00_discard"
triScreenshot "w01_before"
// ---- the walk ------------------------------------------------------------------------
cwBad = 0
cwBadWho = objNull
cwSamples = 0
cwNext = 0
cwBigIn = { private ["_n"]; _n = 0; { if (((getDammage _x) < 1) and ((_x call KJC_fnKind) == "big")) then { _n = _n + 1; cwBadWho = _x } } forEach (nearestObjects [getPos ded, [], 0.2 * KJC_H]); _n }
cwTick = { if (time >= cwNext) then { cwNext = time + 0.5; cwSamples = cwSamples + 1; cwBad = cwBad + (call cwBigIn) } }
cwD = { ded distance cwP0 }
[ded] exec "kj\crush\loop.sqs"
triSimFrames 30
if (true) then { cwI = 0; while { cwI < 6 } do { ded playMove "DedWalk"; cwI = cwI + 1 }; cwT0 = time; cwP0 = getPos ded }
// the tank goes as the toes of his left foot come over it
triSimUntil { call cwTick; (isNull cwTank) or ((getDammage cwTank) >= 1) or (time > cwT0 + 25) }
logInfo format ["CRUSH walk tank gone: t %1 moved %2 footfall %3", time - cwT0, call cwD, KJ_footfall]
[2 * cwHs + cwFa - (call cwD) + 12, -30, 2, 2 * cwHs + cwFa - (call cwD), 0 - cwSt, 5] call cwSee
triSimFrames 12
triScreenshot "w02_tank_as_the_foot_comes"
triSimUntil { call cwTick; ((KJ_footfall select 0) >= 2) or (time > cwT0 + 30) }
[2 * cwHs + cwFa - (call cwD) + 12, -30, 2, 2 * cwHs + cwFa - (call cwD), 0 - cwSt, 5] call cwSee
triSimFrames 4
triScreenshot "w03_foot_on_the_tank"
triSimUntil { call cwTick; ((KJ_footfall select 0) >= 3) or (time > cwT0 + 40) }
[3 * cwHs + cwFa - (call cwD) + 14, -30, 2, 3 * cwHs + cwFa - (call cwD), cwSt, 4] call cwSee
triSimFrames 4
triScreenshot "w04_foot_on_the_jeep"
triSimUntil { call cwTick; ((KJ_footfall select 0) >= 4) or (time > cwT0 + 50) }
[4 * cwHs + cwFa - (call cwD) + 8, -32, 2, 4 * cwHs + cwFa - (call cwD), 0 - cwSt - 4, 4] call cwSee
triSimFrames 20
triScreenshot "w05_men_at_the_footfall"
logInfo format ["CRUSH walk open ground: t %1 moved %2 pos %3 dir %4 footfall %5 stats %6", time - cwT0, call cwD, getPos ded, getDir ded, KJ_footfall, KJ_crushStats]
logInfo format ["CRUSH walk vehicles: tank gone %1 ; jeep gone %2 ; edge jeep dmg %3 moved %4 ; far jeep dmg %5", isNull cwTank, isNull cwJeep, getDammage cwJeepEdge, cwJeepEdge distance cwJeepEdge0, getDammage cwJeepFar]
logInfo format ["CRUSH walk men: under alive %1 ; beside alive %2 dmg %3 move %4 ; far dmg %5", alive cwUnder, alive cwBeside, getDammage cwBeside, getMove cwBeside, getDammage cwFar]
triAssert [(isNull cwTank) or ((getDammage cwTank) >= 1)]
triAssert [(isNull cwJeep) or ((getDammage cwJeep) >= 1)]
triAssertGe [getDammage cwJeepEdge, 1]
triAssertGt [cwJeepEdge distance cwJeepEdge0, 3]
triAssertEq [getDammage cwJeepFar, 0]
triAssert [!(alive cwUnder)]
triAssert [alive cwBeside]
triAssertGt [getDammage cwBeside, 0.05]
triAssertEq [getDammage cwFar, 0]
// no hull stopped him: four footfalls of ground in the time four footfalls take
triAssertGt [call cwD, 3.5 * cwHs]
// ---- the first house -----------------------------------------------------------------
triSimUntil { call cwTick; ((count KJ_crushLog) >= 1) or (time > cwT0 + 70) }
logInfo format ["CRUSH walk house 1 goes: t %1 moved %2 log %3", time - cwT0, call cwD, KJ_crushLog]
[-14, -46, 3, 20, 0, 8] call cwSee
triSimFrames 2
triScreenshot "w06_house1_dust_then_collapse"
cwTc = time
triSimUntil { call cwTick; time > cwTc + 1.1 }
[-14, -46, 3, 20, 0, 8] call cwSee
triSimFrames 2
triScreenshot "w07_house1_coming_down"
triSimUntil { call cwTick; time > cwTc + 2.6 }
[-14, -46, 3, 20, 0, 8] call cwSee
triSimFrames 2
triScreenshot "w08_house1_down"
triSimUntil { call cwTick; ((call cwD) > 68) or (time > cwT0 + 80) }
[26, -40, 2.5, 4, 0, 9] call cwSee
triSimFrames 2
triScreenshot "w09_stepping_into_house1"
logInfo format ["CRUSH walk house 1: t %1 moved %2 pos %3 footfalls %4 stats %5 bad %6", time - cwT0, call cwD, getPos ded, KJ_footfall select 0, KJ_crushStats, cwBad]
// ---- the second house ----------------------------------------------------------------
triSimUntil { call cwTick; ((count KJ_crushLog) >= 2) or (time > cwT0 + 95) }
cwTc = time
triSimUntil { call cwTick; time > cwTc + 0.9 }
[-14, -46, 3, 20, 0, 8] call cwSee
triSimFrames 2
triScreenshot "w10_house2_coming_down"
triSimUntil { call cwTick; ((call cwD) > 93) or (time > cwT0 + 105) }
[30, -36, 2.5, 4, 0, 9] call cwSee
triSimFrames 2
triScreenshot "w11_stepping_into_house2"
triSimUntil { call cwTick; ((call cwD) > 103) or (time > cwT0 + 115) }
triSimUntil { call cwTick; (count KJC_pending) == 0 }
[-48, -52, 40, 16, 0, 0] call cwSee
triSimFrames 2
triScreenshot "w12_the_way_he_came"
// every felled building: the dust led the collapse by a beat
cwNoBeat = 0
{ if (((_x select 2) - (_x select 1)) < 0.4) then { cwNoBeat = cwNoBeat + 1 } } forEach KJ_crushLog
logInfo format ["CRUSH walk end: t %1 moved %2 pos %3 footfalls %4 stats %5 samples %6 bad %7 (%8) log %9", time - cwT0, call cwD, getPos ded, KJ_footfall select 0, KJ_crushStats, cwSamples, cwBad, cwBadWho, count KJ_crushLog]
logInfo format ["CRUSH walk staging: no beat %1 hurt count %2 dmg %3 revived %4 log %5", cwNoBeat, KJ_hurtCount, getDammage ded, KJ_revived, KJ_crushLog]
triAssertGt [call cwD, 95]
triAssertGe [KJ_footfall select 0, 10]
triAssertGe [KJ_crushStats select 0, 2]
triAssertGe [KJ_crushStats select 1, 2]
triAssertEq [cwBad, 0]
triAssertEq [cwNoBeat, 0]
triAssert [alive ded]
triAssertEq [KJ_revived, 0]
triEndTest
