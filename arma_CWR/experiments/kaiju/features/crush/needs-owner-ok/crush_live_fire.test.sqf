// ============================================================================
//  NEEDS-OWNER-OK: this test FIRES WEAPONS (a T-80 shoots him). It is written
//  and has never been run: the standing rule is no gunfire in runs. Ask the
//  owner before running it. It lives outside tests\ so run-crush.ps1 -All
//  never picks it up.
//
//  What it would settle (today these come from reading the engine source only):
//    - which stock rounds raise his damage at all, and by how much per hit
//      (expected with armor 1000, armorStructural 4, geometry radius 22.6 m:
//      Heat125 0.013, Shell125 0.0012, see NOTES.md "Being shot");
//    - that the "Hit" event does not fire for a tank round (it needs 0.05);
//    - that the direction in KJ_hurt points at the tank that fired;
//    - that he is still alive after a full load of tank rounds.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [5060, 4160, 0]
setViewDistance 1500
lfE = createCenter east
lfW = createCenter west
lfGe = createGroup east
lfGw = createGroup west
ded = objNull
"UDDedushka" createUnit [[5060, 3875, 0], lfGe, "ded = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull ded)]
if (true) then { ded setBehaviour "CARELESS"; ded allowFleeing 0; ded setDir 90; ded stop true }
call compile preprocessFile "kj\crush\init.sqf"
lfHits = 0
lfMaxRise = 0
lfLast = 0
ded addEventHandler ["Hit", { lfHits = lfHits + 1 }]
[ded] exec "kj\crush\loop.sqs"
// an M1A1 300 m to the east, on the town's main road
lfTank = "M1Abrams" createVehicle [5360, 3895, 0]
lfCrew = []
{ _x createUnit [[5365, 3890, 0], lfGw, "lfCrew = lfCrew + [this]", 0.8, "PRIVATE"] } forEach ["SoldierWCrew", "SoldierWCrew", "SoldierWCrew"]
triSimFrames 10
if (true) then { (lfCrew select 0) moveInCommander lfTank; (lfCrew select 1) moveInGunner lfTank; (lfCrew select 2) moveInDriver lfTank }
triSimFrames 30
if (true) then { lfGw reveal ded; lfTank doTarget ded; lfTank doFire ded; lfT0 = time }
// watch for two minutes: every rise in damage before hurt.sqs heals it
triSimUntil { if ((getDammage ded) > lfMaxRise) then { lfMaxRise = getDammage ded }; (KJ_hurtCount >= 5) or (time > lfT0 + 120) }
lfBear = (((getPos lfTank) select 0) - ((getPos ded) select 0)) atan2 (((getPos lfTank) select 1) - ((getPos ded) select 1))
if (lfBear < 0) then { lfBear = lfBear + 360 }
logInfo format ["CRUSH live fire: hurt count %1 last %2 expected bearing %3 max damage seen %4 Hit events %5 alive %6 revived %7 tank ammo %8", KJ_hurtCount, KJ_hurt, lfBear, lfMaxRise, lfHits, alive ded, KJ_revived, lfTank ammo "Gun120"]
triAssertGe [KJ_hurtCount, 1]
triAssertLt [abs ((KJ_hurt select 1) - lfBear), 10]
triAssert [alive ded]
triAssertEq [KJ_revived, 0]
triEndTest
