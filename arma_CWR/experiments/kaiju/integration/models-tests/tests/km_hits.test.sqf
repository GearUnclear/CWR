// ============================================================================
//  Dedushka model finishing: HIT REGIONS and PASSING UNDER HIM.
//  WEAPONS ARE FIRED (allowed by the owner on 2026-11-12): one M1A1 fires on
//  him from the front, and shells are let fall on his head and on a foot.
//      models-tests\run-km.ps1 -Name km_hits            (A)
//      models-tests\run-km.ps1 -Name km_hits -Params hound   (D)
//
//  What it settles on the finished models (per-bone hit shape, furnace as its
//  own piece, collision shape with nothing under 0.17 of his height):
//   1. a tank round from the front lands in the FURNACE (the combat feature's
//      own word for the place of the hit)
//   2. a shell that falls on his skull is reported as HEAD
//   3. a shell that bursts at a foot is reported as LEGS, and a shell let fall on
//      a leg's own hit box strikes the leg itself (the hit shape reaches the ground)
//   4. two men walk and a tank drives THROUGH where his legs stand, and he does
//      not move
//  Soft checks; the verdict is the KMH RESULT line.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4900, 4160, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.25
0 setFog 0
setViewDistance 1500
showCinemaBorder false
call compile preprocessFile "kjd_params.sqf"
if (isNil "KJD_class") then { KJD_class = "UDDedushka" }
kmR = createCenter resistance
kmW = createCenter west
kmC = createCenter civilian
if (true) then { resistance setFriend [west, 0]; west setFriend [resistance, 0] }
kmGr = createGroup resistance
kmGw = createGroup west
kmGc = createGroup civilian
kmGc2 = createGroup civilian
kmGc3 = createGroup civilian
kaiju = objNull
KJD_class createUnit [[4900, 4004, 0], kmGr, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
kmFails = []
kmChk = { if (!(_this select 0)) then { kmFails = kmFails + [_this select 1]; logInfo format ["KMH CHECK FAILED: %1", _this select 1] } else { logInfo format ["KMH ok: %1", _this select 1] } }
kmFired = []
KJCB_LOG = 80
if (true) then { kaiju setDir 270; kaiju setPos [4900, 4004, 0]; KJ_brainAuto = false; KJ_ready = false; [kaiju, "idle"] exec "kj\start.sqs" }
triSimUntil { KJ_ready }
triSimFrames 60
if (isNil "KJ_woundHealth") then { KJ_woundHealth = -1; KJ_combatStats = [0, 0, 0, 0, 0, 0, 0]; KJ_combatByAmmo = []; KJ_combatByZone = [0, 0, 0, 0, 0]; KJ_combatLast = [0, "", "", 0, 0, objNull] }
KJD_probe = "HeliHEmpty" createVehicle [4900, 4100, 0]
kmView = { private ["_g"]; KJD_probe setPos [_this select 0, _this select 1, 0]; _g = ((getPosASL KJD_probe) select 2) + (_this select 2); [_this select 0, _g, _this select 1, ((getPosASL kaiju) select 0) - (_this select 0), ((getPosASL kaiju) select 2) + (_this select 3) - _g, ((getPosASL kaiju) select 1) - (_this select 1)] }
triSetView ([4838, 3972, 18, 24] call kmView)
triSimFrames 8
triScreenshot "00_warmup_discard"
kmHeadAt = [[kaiju, "kjHeadRight", 0] call KJCB_fnCfg, [kaiju, "kjHeadFwd", 10] call KJCB_fnCfg, [kaiju, "kjHeadUp", 36] call KJCB_fnCfg]
// where a foot's hit box is alone under the sky, in his frame [right, forward, up]: A's left foot, D's lizard foot
kmFootAt = [-9.7, -3.5, 0]
if ((typeOf kaiju) == "UDDedushkaHound") then { kmFootAt = [-12, -3, 0] }
// a point of a leg's hit box with free air straight above it, and the height to let a shell go from
// (worked out on the built hit shape: A's left foot under the chest, D's lizard shin under the sky)
kmLegAt = [-6.0, 5.8, 0]
kmLegDrop = 12
if ((typeOf kaiju) == "UDDedushkaHound") then { kmLegAt = [-8.0, -3.0, 0]; kmLegDrop = 26 }
logInfo format ["KMH start: creature %1 side %2, native %3, combat %4, health %5, head point %6, foot point %7, regions: legs under %8, head r %9, furnace r %10", typeOf kaiju, side kaiju, KJ_native, KJ_hasCombat, KJ_woundHealth, kmHeadAt, kmFootAt, KJCB_legsUp, KJCB_headR, KJCB_furnaceR]
// ---- 1. the furnace: an M1A1 300 m in front of him ---------------------------------------
kmTank = "M1Abrams" createVehicle [4600, 4010, 0]
kmCrew = []
{ _x createUnit [[4594, 4004, 0], kmGw, "kmCrew = kmCrew + [this]", 0.8, "PRIVATE"] } forEach ["SoldierWCrew", "SoldierWCrew", "SoldierWCrew"]
triSimFrames 10
if (true) then { (kmCrew select 0) moveInCommander kmTank; (kmCrew select 1) moveInGunner kmTank; (kmCrew select 2) moveInDriver kmTank; kmTank setDir 90; kmTank addEventHandler ["Fired", { if ((count kmFired) < 60) then { kmFired = kmFired + [[time, _this select 1, _this select 4]] } }] }
triSimFrames 30
if (true) then { kmZ0 = +KJ_combatByZone; kmGw reveal kaiju; kmTank reveal kaiju; kmGw setCombatMode "RED"; kmT0 = time }
triSimUntil { (({ (_x select 1) == "Gun120" } count kmFired) >= 3) || (time > kmT0 + 100) }
triSimFrames 90
triScreenshot "H01_tank_rounds_in_the_furnace"
if (true) then { kmZ1 = +KJ_combatByZone; kmLastTank = +KJ_combatLast; kmGw setCombatMode "BLUE"; { deleteVehicle _x } forEach kmCrew; deleteVehicle kmTank }
logInfo format ["KMH tank: rounds fired %1, hits by zone before %2 after %3 (furnace, head, body, legs, unknown), last hit %4", count kmFired, kmZ0, kmZ1, kmLastTank]
[(count kmFired) >= 1, "the tank fired on him"] call kmChk
[((kmZ1 select 0) - (kmZ0 select 0)) >= 1, "a tank round from the front was reported in the furnace"] call kmChk
triSimFrames 120
// ---- 2 and 3. shells let fall on the skull and on a foot ------------------------------------
kmDrop = { private ["_w"]; _w = [kaiju, [(_this select 0) select 0, (_this select 0) select 1, ((_this select 0) select 2) + (_this select 1)]] call KJCB_fnToWorld; kmShell = (_this select 2) camCreate _w; logInfo format ["KMH drop %1 at %2 (his frame %3 + %4 m)", _this select 2, _w, _this select 0, _this select 1] }
kmWait = { kmN0 = (KJ_combatStats select 1) + (KJ_combatStats select 2); kmTd = time }
triSetView ([4852, 3972, 30, 35] call kmView)
if (true) then { kmZa = +KJ_combatByZone; call kmWait; [kmHeadAt, 9, "Heat120"] call kmDrop }
triSimUntil { (((KJ_combatStats select 1) + (KJ_combatStats select 2)) > kmN0) || (time > kmTd + 8) }
triSimFrames 20
triScreenshot "H02_a_shell_on_the_skull"
if (true) then { kmZb = +KJ_combatByZone; kmLastHead = +KJ_combatLast }
if ((kmZb select 1) == (kmZa select 1)) then { call kmWait; [kmHeadAt, 9, "Shell120"] call kmDrop }
triSimUntil { (((KJ_combatStats select 1) + (KJ_combatStats select 2)) > kmN0) || (time > kmTd + 8) }
triSimFrames 20
if (true) then { kmZb = +KJ_combatByZone; kmLastHead = +KJ_combatLast }
logInfo format ["KMH head: by zone before %1 after %2, last hit %3", kmZa, kmZb, kmLastHead]
[((kmZb select 1) - (kmZa select 1)) >= 1, "a shell on the skull was reported as head"] call kmChk
triSimFrames 90
triSetView ([4878, 3988, 9, 5] call kmView)
if (true) then { kmZc = +KJ_combatByZone; call kmWait; [kmFootAt, 14, "Heat120"] call kmDrop }
triSimUntil { (((KJ_combatStats select 1) + (KJ_combatStats select 2)) > kmN0) || (time > kmTd + 8) }
triSimFrames 20
triScreenshot "H03_a_shell_on_the_foot"
if (true) then { kmZd = +KJ_combatByZone; kmLastLeg = +KJ_combatLast }
if ((kmZd select 3) == (kmZc select 3)) then { call kmWait; [kmFootAt, 14, "Shell120"] call kmDrop }
triSimUntil { (((KJ_combatStats select 1) + (KJ_combatStats select 2)) > kmN0) || (time > kmTd + 8) }
triSimFrames 20
if (true) then { kmZd = +KJ_combatByZone; kmLastLeg = +KJ_combatLast }
logInfo format ["KMH leg: by zone before %1 after %2, last hit %3", kmZc, kmZd, kmLastLeg]
[((kmZd select 3) - (kmZc select 3)) >= 1, "a shell that burst at the foot was reported as legs"] call kmChk
triSimFrames 90
// a shell let fall on the leg's own hit box (a round given a level speed in the air did not fly: run hits-a2)
if (true) then { kmZe = +KJ_combatByZone; call kmWait; [kmLegAt, kmLegDrop, "Heat120"] call kmDrop }
triSimUntil { (((KJ_combatStats select 1) + (KJ_combatStats select 2)) > kmN0) || (time > kmTd + 8) }
triSimFrames 20
triScreenshot "H04_a_shell_on_the_leg"
if (true) then { kmZf = +KJ_combatByZone; kmLastShin = +KJ_combatLast }
logInfo format ["KMH leg, direct: by zone before %1 after %2, last hit %3", kmZe, kmZf, kmLastShin]
[(((kmZf select 3) - (kmZe select 3)) >= 1) && ((kmLastShin select 3) >= 500) && ((kmLastShin select 2) == "legs"), "a shell on the leg's hit box struck the leg itself (a direct hit at full weight, reported as legs)"] call kmChk
triSimFrames 90
// ---- 4. passing under him: the crush loop is stopped so that nothing is trodden on -------------
if (true) then { KJ_crushOn = false; KJ_brainOn = false }
triSimFrames 60
kmP0 = getPos kaiju
kmMoved = 0
kmTrack = { private ["_d"]; _d = sqrt ((((getPos kaiju) select 0) - (kmP0 select 0)) ^ 2 + (((getPos kaiju) select 1) - (kmP0 select 1)) ^ 2); if (_d > kmMoved) then { kmMoved = _d } }
kmMan1 = objNull
kmMan2 = objNull
"Civilian" createUnit [[4868, 4004, 0], kmGc, "kmMan1 = this", 0.6, "PRIVATE"]
"Civilian" createUnit [[4868, 4010.2, 0], kmGc2, "kmMan2 = this", 0.6, "PRIVATE"]
triSimFrames 10
if (true) then { { _x setCaptive true; _x setBehaviour "CARELESS"; _x setSpeedMode "FULL"; _x allowDammage false } forEach [kmMan1, kmMan2]; kmMan1 doMove [4936, 4004, 0]; kmMan2 doMove [4936, 4010.2, 0]; kmT1 = time }
triSetView ([4876, 3990, 10, 3] call kmView)
triSimUntil { call kmTrack; (((getPos kmMan1) select 0) > 4899) || (time > kmT1 + 60) }
triScreenshot "P01_men_under_him"
triSimUntil { call kmTrack; ((((getPos kmMan1) select 0) > 4928) && (((getPos kmMan2) select 0) > 4928)) || (time > kmT1 + 110) }
logInfo format ["KMH men: man between the feet at %1, man through the right foot at %2, after %3 s; he moved %4 m at most", getPos kmMan1, getPos kmMan2, time - kmT1, kmMoved]
[((getPos kmMan1) select 0) > 4925, "a man walked between his feet to the far side"] call kmChk
[((getPos kmMan2) select 0) > 4925, "a man walked through where his right foot stands to the far side"] call kmChk
kmMovedMen = kmMoved
// a tank through where his right foot stands (his right foot is at x 4897.6, y 4010.2): an empty T-80
// kept rolling east at 9 m/s (a driver steers round him, as run hits-a1 showed, so nobody drives it)
kmT80 = "T80" createVehicle [4856, 4010.2, 0]
triSimFrames 30
if (true) then { kmT80 allowDammage false; kmT80 setDir 90; kmT2 = time; kmNear = 999 }
triSimUntil { call kmTrack; kmT80 setVelocity [9, 0, 0]; kmDn = sqrt ((((getPos kmT80) select 0) - 4897.6) ^ 2 + (((getPos kmT80) select 1) - 4010.2) ^ 2); if (kmDn < kmNear) then { kmNear = kmDn }; (((getPos kmT80) select 0) > 4930) || (time > kmT2 + 40) }
triSetView ([4876, 3990, 10, 3] call kmView)
triSimFrames 4
triScreenshot "P02_tank_past_his_foot"
logInfo format ["KMH tank: at %1 after %2 s, nearest to the middle of his right foot %3 m; he moved %4 m at most in the whole pass (men alone %5); his position %6 from %7", getPos kmT80, time - kmT2, kmNear, kmMoved, kmMovedMen, getPos kaiju, kmP0]
[(((getPos kmT80) select 0) > 4925) && (kmNear < 2.5), "a tank rolled through where his right foot stands to the far side"] call kmChk
[kmMoved < 0.5, "he did not move while men and a tank passed under him (under half a metre)"] call kmChk
[alive kaiju, "he is alive after it"] call kmChk
logInfo format ["KMH end: health %1, combat stats %2, by ammo %3, by zone %4", KJ_woundHealth, KJ_combatStats, KJ_combatByAmmo, KJ_combatByZone]
logInfo format ["KMH RESULT: %1 checks failed: %2", count kmFails, kmFails]
if (true) then { KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false; KJ_combatOn = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
triEndTest
