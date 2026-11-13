// ============================================================================
//  Dedushka beam test (experiment). Two beams on the south edge of Morton
//  (Everon), one by day and one by night, with screenshots of every phase.
//
//  The day beam is the one the asserts are about. He stands at [5055, 3830]
//  facing a little west of north (so the cut starts from the west) and is told
//  to burn the point 200 m north of him, so the cut
//  runs from 28 degrees west of north to 28 degrees east of it, 186 to 214 m
//  out. On that path stand three soldiers and an empty T-80. A fourth soldier
//  stands far outside the arc and a fifth right under the beam where it is
//  still 20 m up: both must live. An empty Mi-17 hangs 90 m up to the east
//  (held there by hold.sqs): the beam must tilt up and bring it down.
//
//  Needs @uddedushka and @kj_beam. Run through features/beam/run-beam.ps1.
//  Quiet: nobody is armed, nobody fires, every death is the script's.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [5450, 4250, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.2
0 setFog 0
setViewDistance 3500
showCinemaBorder false
bbC = createCenter civilian
bbG = createGroup civilian
bbE = createCenter east
bbEG = createGroup east
ded = objNull
bbAt = [5055, 3830, 0]
"UDDedushka" createUnit [bbAt, bbG, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
if (true) then { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; ded setBehaviour "CARELESS"; ded stop true; ded setDir 350; ded setPos bbAt; ded allowDammage false }
// The people and the armour in the way. Men 0 to 2 stand on the cut, man 3 far
// outside it, man 4 under the beam where it is still high.
bbMen = []
bbMk = { "SoldierEB" createUnit [_this, bbEG, "bbMen = bbMen + [this]", 0.5, "PRIVATE"] }
[5008.3, 4017.3, 0] call bbMk
[5055, 4030, 0] call bbMk
[5105.1, 4030.9, 0] call bbMk
[5242.9, 3898.4, 0] call bbMk
[5055, 3890, 0] call bbMk
triSimFrames 5
triAssertEq [count bbMen, 5]
{ _x setCaptive true; _x disableAI "MOVE"; _x setCombatMode "BLUE"; _x setBehaviour "CARELESS"; _x stop true; removeAllWeapons _x } forEach bbMen
bbTank = "T80" createVehicle [5078.8, 4032.0, 0]
bbHeli = "Mi17" createVehicle [5306, 3897, 0]
[bbHeli, [5306, 3897, 90]] exec "hold.sqs"
// The nearest house to each man on the cut, to see what the beam does to buildings.
bbHouses = []
{ bbHouses = bbHouses + [nearestBuilding _x] } forEach [bbMen select 0, bbMen select 1, bbMen select 2]
bbHouseLog = { private ["_s", "_i"]; _s = ""; _i = 0; { _s = _s + format [" [house %1 at %2, %3 m from man, damage %4]", _i, getPos _x, _x distance (bbMen select _i), damage _x]; _i = _i + 1 } forEach bbHouses; _s }
bbAlive = { private ["_n"]; _n = 0; { if (alive _x) then { _n = _n + 1 } } forEach _this; _n }
// Camera: HUD off, lens by bbFov, framing by triSetView.
bbProbe = "HeliHEmpty" createVehicle [5055, 4200, 0]
bbCam = "camera" camCreate [5055, 3700, 30]
bbCam cameraEffect ["internal", "back"]
bbFov = { bbCam camSetTarget [5055, 3900, 30]; bbCam camSetFov _this; bbCam camCommit 0 }
0.7 call bbFov
// bbView: [camera x, camera y, eye height above its ground or the sea, look-at x, look-at y, look-at height above sea level]
bbView = { private ["_g"]; bbProbe setPos [_this select 0, _this select 1, 0]; _g = (getPosASL bbProbe) select 2; if (_g < 2.75) then { _g = 0.3 }; [_this select 0, _g + (_this select 2), _this select 1, (_this select 3) - (_this select 0), (_this select 5) - (_g + (_this select 2)), (_this select 4) - (_this select 1)] }
triSimFrames 150
logInfo format ["BEAMTEST placed: ded asl %1 men %2 tank %3 heli %4 man0 %5 man1 %6 man2 %7", getPosASL ded, count bbMen, getPosASL bbTank, getPosASL bbHeli, getPos (bbMen select 0), getPos (bbMen select 1), getPos (bbMen select 2)]
logInfo format ["BEAMTEST houses before:%1", call bbHouseLog]
triSetView ([4950, 3800, 1.7, 5055, 3835, 46] call bbView)
triSimFrames 6
triScreenshot "00_warmup_discard"
triScreenshot "d00_before_110m"
// ---------------------------------------------------------------- day beam --
[ded, [5055, 4030, 0]] exec "kj\beam\fire.sqs"
triSimFrames 4
triAssert [KJ_beamActive]
triAssertEq [KJ_beamPhase, "still"]
triSimUntil { KJ_beamPhase == "charge" }
if (true) then { bbT0 = time; logInfo format ["BEAMTEST charge began %1", time] }
triSimUntil { time > bbT0 + 2.5 }
triScreenshot "d01_charge_early_110m"
triSimUntil { time > bbT0 + 6.0 }
triScreenshot "d02_charge_late_110m"
// The wait hurts nobody.
triAssertEq [bbMen call bbAlive, 5]
triAssert [alive bbTank]
triSimUntil { KJ_beamPhase == "fire" }
if (true) then { bbT0 = time; logInfo format ["BEAMTEST fire began %1 segs %2", time, count KJ_beamSegs] }
triAssertEq [count KJ_beamSegs, 16]
triSetView ([5330, 3900, 1.7, 5030, 3930, 36] call bbView)
triSimUntil { time > bbT0 + 0.7 }
triScreenshot "d03_fire_hold_side_280m"
triSetView ([4960, 3960, 1.7, 5060, 3990, 30] call bbView)
triSimUntil { time > bbT0 + 4.0 }
triScreenshot "d04_fire_sweep_from_west_field"
triSetView ([5400, 2890, 1.7, 5055, 3900, 44] call bbView)
triSimUntil { time > bbT0 + 7.5 }
triScreenshot "d05_fire_far_1km"
triSetView ([5135, 4062, 1.7, 5090, 4015, 24] call bbView)
triSimUntil { time > bbT0 + 11.2 }
triScreenshot "d06_fire_near_impact"
triSetView ([4900, 3760, 70, 5055, 4000, 20] call bbView)
triSimUntil { time > bbT0 + 14.5 }
triScreenshot "d07_fire_high_wide"
triSimUntil { KJ_beamPhase != "fire" }
if (true) then { bbT0 = time; logInfo format ["BEAMTEST ground cut over %1 phase %2 kills %3 wrecks %4 razed %5 queue %6 fires %7", time, KJ_beamPhase, KJ_beamKills, KJ_beamWrecks, KJ_beamRazed, count KJ_beamQueue, KJ_beamFires] }
triAssertEq [KJ_beamPhase, "sky"]
triSetView ([5180, 3600, 1.7, 5180, 3860, 62] call bbView)
triSimUntil { (time > bbT0 + 2.2) || (KJ_beamPhase != "sky") }
triScreenshot "d08_sky_tilting"
triSimUntil { (KJ_beamAirKills > 0) || (KJ_beamPhase != "sky") }
triScreenshot "d09_sky_hit"
triSimUntil { KJ_beamPhase == "spent" }
if (true) then { bbT0 = time; logInfo format ["BEAMTEST out %1 kills %2 wrecks %3 air %4 razed %5 queue %6 fires %7 men alive %8 tank %9 heli %10", time, KJ_beamKills, KJ_beamWrecks, KJ_beamAirKills, KJ_beamRazed, count KJ_beamQueue, KJ_beamFires, bbMen call bbAlive, alive bbTank, alive bbHeli] }
triAssert [!KJ_beamActive]
triAssert [KJ_beamSpentUntil > time + 60]
triAssertEq [KJ_beamShots, 1]
triAssertEq [count KJ_beamSegs, 0]
triAssert [!(alive (bbMen select 0))]
triAssert [!(alive (bbMen select 1))]
triAssert [!(alive (bbMen select 2))]
triAssert [alive (bbMen select 3)]
triAssert [alive (bbMen select 4)]
triAssert [!(alive bbTank)]
triAssert [!(alive bbHeli)]
triAssertEq [KJ_beamAirKills, 1]
triAssert [KJ_beamKills >= 2]
triAssert [KJ_beamFires >= 4]
triSetView ([4950, 3800, 1.7, 5055, 3835, 46] call bbView)
triSimUntil { time > bbT0 + 4 }
triScreenshot "d10_spent_chest_smoke"
// A second order while he is spent is refused.
[ded, [5055, 4030, 0]] exec "kj\beam\fire.sqs"
triSimFrames 30
triAssertEq [KJ_beamShots, 1]
triAssert [!KJ_beamActive]
triAssertEq [KJ_beamPhase, "spent"]
triSetView ([5055, 3900, 80, 5055, 4030, 16] call bbView)
triSimUntil { time > bbT0 + 10 }
triScreenshot "d11_trail_from_above"
triSetView ([5055, 4085, 1.7, 5050, 4030, 22] call bbView)
triSimUntil { time > bbT0 + 22 }
triScreenshot "d12_trail_ground_level"
logInfo format ["BEAMTEST houses after:%1", call bbHouseLog]
logInfo format ["BEAMTEST day end %1: razed %2 queue %3 fires %4 sites %5", time, KJ_beamRazed, count KJ_beamQueue, KJ_beamFires, count KJ_beamSites]
// -------------------------------------------------------------- night beam --
// Second beam in the dark, from one point to another, for the look. A second
// Mi-17 hangs to the north-east.
setDate [1985, 6, 10, 23, 30]
bbHeli2 = "Mi17" createVehicle [5290, 4040, 0]
[bbHeli2, [5290, 4040, 100]] exec "hold.sqs"
triSimFrames 250
triSetView ([4950, 3800, 1.7, 5055, 3835, 46] call bbView)
triSimFrames 6
triScreenshot "n00_spent_night_110m"
KJ_beamIgnoreSpent = true
[ded, [4990, 4000, 0], [5130, 3990, 0]] exec "kj\beam\fire.sqs"
triSimFrames 4
triAssert [KJ_beamActive]
triSimUntil { KJ_beamPhase == "charge" }
if (true) then { bbT0 = time }
triSimUntil { time > bbT0 + 1.5 }
triScreenshot "n01_charge_early_110m"
triSimUntil { time > bbT0 + 4.5 }
triScreenshot "n02_charge_mid_110m"
triSimUntil { time > bbT0 + 5.8 }
triScreenshot "n03_charge_late_110m"
triSimUntil { KJ_beamPhase == "fire" }
if (true) then { bbT0 = time }
triSimUntil { time > bbT0 + 0.8 }
triScreenshot "n04_fire_hold_110m"
triSetView ([4800, 3900, 1.7, 5055, 3930, 36] call bbView)
triSimUntil { time > bbT0 + 3.0 }
triScreenshot "n05_fire_side_west_260m"
triSetView ([5400, 2890, 1.7, 5055, 3900, 44] call bbView)
triSimUntil { time > bbT0 + 5.5 }
triScreenshot "n06_fire_far_1km"
triSetView ([5700, 1950, 1.7, 5055, 3900, 44] call bbView)
triSimUntil { time > bbT0 + 8.0 }
triScreenshot "n07_fire_far_2km"
triSetView ([4900, 3760, 70, 5055, 4000, 20] call bbView)
triSimUntil { time > bbT0 + 10.5 }
triScreenshot "n08_fire_high_wide"
triSetView ([5135, 4062, 1.7, 5090, 4000, 24] call bbView)
triSimUntil { (time > bbT0 + 12.5) || (KJ_beamPhase != "fire") }
triScreenshot "n09_fire_near_impact"
triSimUntil { KJ_beamPhase != "fire" }
if (true) then { bbT0 = time }
triSetView ([5180, 3600, 1.7, 5180, 3900, 70] call bbView)
triSimUntil { (time > bbT0 + 1.2) || (KJ_beamPhase != "sky") }
triScreenshot "n10_sky_tilting"
triSimUntil { KJ_beamPhase == "spent" }
if (true) then { bbT0 = time; logInfo format ["BEAMTEST night out %1 shots %2 air %3 heli2 %4 razed %5 fires %6", time, KJ_beamShots, KJ_beamAirKills, alive bbHeli2, KJ_beamRazed, KJ_beamFires] }
triAssertEq [KJ_beamShots, 2]
triAssert [!(alive bbHeli2)]
triSetView ([4950, 3800, 1.7, 5055, 3835, 46] call bbView)
triSimUntil { time > bbT0 + 4 }
triScreenshot "n11_spent_dull_red"
triSetView ([5055, 3900, 80, 5055, 4030, 16] call bbView)
triSimUntil { time > bbT0 + 9 }
triScreenshot "n12_trail_from_above"
triSetView ([5055, 4085, 1.7, 5050, 4030, 22] call bbView)
triSimUntil { time > bbT0 + 14 }
triScreenshot "n13_trail_ground_level"
logInfo format ["BEAMTEST end %1: kills %2 wrecks %3 air %4 razed %5 fires %6 sites %7 ded alive %8", time, KJ_beamKills, KJ_beamWrecks, KJ_beamAirKills, KJ_beamRazed, KJ_beamFires, count KJ_beamSites, alive ded]
triAssert [alive ded]
triEndTest
