// ============================================================================
//  Dedushka integration: ONE TANK FIRES ON HIM (WEAPONS ARE FIRED; allowed by
//  the owner on 2026-11-12). This is the crush feature's unrun live-fire test
//  (features\crush\needs-owner-ok\crush_live_fire), brought up to date in the
//  integration: the one script set, the combat feature in place of the heal
//  loop, the creature config of today (side Resistance, armor 700,
//  armorStructural 17). An M1A1 300 m in front of him on the open road west of
//  the town, on its own. (First try, run tank1: the tank stood east of him with
//  the town's row between them and not one of its rounds reached him.)
//      run-kj.ps1 -Name kj_livetank
//
//  What it settles, in the integrated set:
//   - a tank round raises the hit signal, and the signal's bearing points at
//     the tank that fired
//   - the engine's "Hit" event does not fire for a tank round (it needs 0.05)
//   - what a round costs him in the combat feature's health (its notes: 2.5
//     points of 320 for a HEAT round into the furnace)
//   - that the engine's own damage value is only the messenger (wiped), and
//     that he is alive after a minute of it
//  Soft checks; the verdict is the KJT RESULT line.
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
kjtR = createCenter resistance
kjtW = createCenter west
if (true) then { resistance setFriend [west, 0]; west setFriend [resistance, 0] }
kjtGr = createGroup resistance
kjtGw = createGroup west
kaiju = objNull
KJD_class createUnit [[4900, 4004, 0], kjtGr, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
kjFails = []
kjChk = { if (!(_this select 0)) then { kjFails = kjFails + [_this select 1]; logInfo format ["KJT CHECK FAILED: %1", _this select 1] } else { logInfo format ["KJT ok: %1", _this select 1] } }
kjtHitEv = 0
kjtFired = []
kjtMaxRise = 0
if (true) then { kaiju setDir 270; kaiju setPos [4900, 4004, 0]; kaiju addEventHandler ["Hit", { kjtHitEv = kjtHitEv + 1 }]; KJ_brainAuto = false; KJ_ready = false; [kaiju, "idle"] exec "kj\start.sqs" }
triSimUntil { KJ_ready }
triSimFrames 60
if (isNil "KJ_woundHealth") then { KJ_woundHealth = -1; KJ_combatStats = [0, 0, 0, 0, 0, 0, 0]; KJ_combatByAmmo = []; KJ_combatByZone = [] }
// an M1A1 300 m to the west, in front of him on open ground, with its crew
kjtTank = "M1Abrams" createVehicle [4600, 4010, 0]
kjtCrew = []
{ _x createUnit [[4594, 4004, 0], kjtGw, "kjtCrew = kjtCrew + [this]", 0.8, "PRIVATE"] } forEach ["SoldierWCrew", "SoldierWCrew", "SoldierWCrew"]
triSimFrames 10
if (true) then { (kjtCrew select 0) moveInCommander kjtTank; (kjtCrew select 1) moveInGunner kjtTank; (kjtCrew select 2) moveInDriver kjtTank; kjtTank setDir 90; kjtTank addEventHandler ["Fired", { if ((count kjtFired) < 60) then { kjtFired = kjtFired + [[time, _this select 1, _this select 4]] } }] }
triSimFrames 30
KJD_probe = "HeliHEmpty" createVehicle [4900, 4100, 0]
kjtView = { private ["_g"]; KJD_probe setPos [_this select 0, _this select 1, 0]; _g = ((getPosASL KJD_probe) select 2) + (_this select 2); [_this select 0, _g, _this select 1, ((getPosASL kaiju) select 0) - (_this select 0), ((getPosASL kaiju) select 2) + (_this select 3) - _g, ((getPosASL kaiju) select 1) - (_this select 1)] }
triSetView ([4570, 4030, 4, 24] call kjtView)
triSimFrames 8
triScreenshot "00_warmup_discard"
logInfo format ["KJT start: creature %1 side %2, native %3, combat feature %4, unkillable %5, health %6, tank %7 crew %8", typeOf kaiju, side kaiju, KJ_native, KJ_hasCombat, KJ_unkillable, KJ_woundHealth, typeOf kjtTank, count (crew kjtTank)]
if (true) then { kjtGw reveal kaiju; kjtTank reveal kaiju; kjtGw setCombatMode "RED"; kjtT0 = time; kjtH0 = KJ_woundHealth }
// a minute of it; the highest value the engine's damage ever showed between two reads
triSimUntil { if ((getDammage kaiju) > kjtMaxRise) then { kjtMaxRise = getDammage kaiju }; ((count kjtFired) >= 1) || (time > kjtT0 + 60) }
if (true) then { kjtShotAt = time - kjtT0 }
triSimFrames 40
triScreenshot "T01_the_tank_fires"
triSimUntil { if ((getDammage kaiju) > kjtMaxRise) then { kjtMaxRise = getDammage kaiju }; (({ (_x select 1) == "Gun120" } count kjtFired) >= 6) || (time > kjtT0 + 120) }
triSimFrames 60
triSetView ([4840, 3960, 3, 26] call kjtView)
triSimFrames 6
triScreenshot "T02_after_six_rounds"
kjtBear = [getPos kaiju, getPos kjtTank] call KJB_fnBear
logInfo format ["KJT end: first round %1 s after the reveal, rounds fired %2, hurt count %3, last hurt %4, bearing to the tank %5, Hit events %6, highest engine damage seen %7, engine damage now %8", kjtShotAt, count kjtFired, KJ_hurtCount, KJ_hurt, kjtBear, kjtHitEv, kjtMaxRise, getDammage kaiju]
kjtGun = { (_x select 1) == "Gun120" } count kjtFired
logInfo format ["KJT end: health %1 to %2, points per main gun round %3 (%11 main gun rounds), wound stage %4, combat stats %5, by ammo %6, by zone %7, state %8, alive %9, rounds (time, weapon, ammo) %10", kjtH0, KJ_woundHealth, if (kjtGun > 0) then { (kjtH0 - KJ_woundHealth) * 320 / kjtGun } else { -1 }, call KJ_fnWound, KJ_combatStats, KJ_combatByAmmo, KJ_combatByZone, KJ_state, alive kaiju, kjtFired, kjtGun]
[(count kjtFired) >= 1, "the tank fired on him by its own choice"] call kjChk
[KJ_hurtCount >= 1, "a tank round raised the hit signal"] call kjChk
[(KJ_hurtCount == 0) || ((abs ([KJ_hurt select 1, kjtBear] call KJB_fnAng)) < 10), "the signal's bearing points at the tank (within 10 degrees)"] call kjChk
[(KJ_hurtCount == 0) || ((KJ_hurt select 4) == kjtTank), "the signal names the tank"] call kjChk
[kjtHitEv == 0, "the engine's Hit event did not fire for a tank round"] call kjChk
[(!KJ_hasCombat) || (KJ_woundHealth < kjtH0), "the rounds cost him health (the combat feature's)"] call kjChk
[alive kaiju, "he is alive after it"] call kjChk
logInfo format ["KJT RESULT: %1 checks failed: %2", count kjFails, kjFails]
if (true) then { KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false; KJ_combatOn = false }
triSimFrames 30
if (KJ_native) then { kjClear kaiju }
triSimFrames 5
triEndTest
