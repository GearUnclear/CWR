// ============================================================================
//  One-shot probe for the aimed-fire rework of the narrative shoots.
//
//  QUESTION: can a setCaptive+disableAI-MOVE soldier be made to aim LEVEL at a
//  human target and fire real rounds, by revealing a captive enemy to his group
//  and ordering `doFire`?  Engine reading says yes:
//    * setCaptive only affects how OTHERS classify the unit (TargetSide ->
//      TCivilian, VehicleAIDiag.cpp:1165); it does not stop the captive from
//      engaging targets himself.
//    * disableAI "MOVE" (DAMove) is only checked in the pilot/movement code
//      (VehicleAIPilot.cpp:328,639); targeting is DATarget/DAAutoTarget.
//    * `doFire` (silent path, AIGroupCmd.cpp:909-916) does AssignTarget +
//      EnableFireTarget; the enable-fire target bypasses the IsEnemy gate in
//      SelectFireWeapon (TargetFire.cpp:1795), so a CIVILIAN-classified
//      (captive) target is still shot at.  The unit then aims via AimWeaponAI
//      (level, at the man) instead of Man::AimWeaponForceFire's dir[1]=10
//      skyward salute that the bare `fire` command produces.
//  RISK the probe must measure: CheckFriendlyFire (VehicleAICombat.cpp:1538)
//  blocks the shot if a friendly sits within `radius` of the firing lane -
//  radius 2.0 m for targets >= 50 m away but only 1.0 m under 50 m.  The
//  shoots' staggered line puts each squadmate ~1.5 m off his neighbour's lane,
//  so 40 m engagements should fire and 90 m ones should be blocked.
//
//  Trio A: scene-1 noe spacing (gap 1.7 / gapZ -0.5), targets 40 m up the road.
//  Trio B: same spacing, targets 90 m out - expected to be lane-blocked.
//  Payload comes back through the deliberately-failing assert at the end.
// ============================================================================

triSimUntil { alive player }

setViewDistance 2500
0 setOvercast 0.05
0 setFog 0.02
showCinemaBorder false

player setPos [7000, 9000, 0]
player setCaptive true

ssCam = "camera" camCreate [7000, 9000, 40]
ssCam cameraEffect ["internal", "back"]
triSimFrames 20

createCenter east
createCenter resistance

ssAim = {
    ssT = getPosASL ssTgt;
    ssCx = (ssT select 0) + (ssOff select 0);
    ssCy = (ssT select 2) + (ssOff select 1);
    ssCz = (ssT select 1) + (ssOff select 2);
    triSetView [ssCx, ssCy, ssCz, (ssT select 0) - ssCx, ((ssT select 2) + ssAimH) - ssCy, (ssT select 1) - ssCz]
}

// --- trio A shooters: forest road, bearing 13, scene-1 spacing -------------
ssGrpA = createGroup resistance
call { ssI = 0; while "ssI < 3" do { (["SoldierGB", "SoldierGMG", "SoldierGLAW"] select ssI) createUnit [[9727 + ssI * 1.7, 12452 + ssI * 0.5, 0], ssGrpA, "", 0.4, "PRIVATE"]; ssI = ssI + 1 } }
triSimUntil { (count (units ssGrpA)) >= 3 }

// --- trio A targets: 40 m up the road (offset 40*[sin13,cos13] = [9,39]) ---
ssFoeA = createGroup east
call { ssI = 0; while "ssI < 3" do { "SoldierEB" createUnit [[9736 + ssI * 1.7, 12491 + ssI * 0.5, 0], ssFoeA, "", 0.4, "PRIVATE"]; ssI = ssI + 1 } }
triSimUntil { (count (units ssFoeA)) >= 3 }

// --- trio B shooters + targets, 90 m engagement, further south on the road -
ssGrpB = createGroup resistance
call { ssI = 0; while "ssI < 3" do { (["SoldierGB", "SoldierGMG", "SoldierGLAW"] select ssI) createUnit [[9722 + ssI * 1.7, 12420 + ssI * 0.5, 0], ssGrpB, "", 0.4, "PRIVATE"]; ssI = ssI + 1 } }
triSimUntil { (count (units ssGrpB)) >= 3 }
ssFoeB = createGroup east
call { ssI = 0; while "ssI < 3" do { "SoldierEB" createUnit [[9742 + ssI * 1.7, 12508 + ssI * 0.5, 0], ssFoeB, "", 0.4, "PRIVATE"]; ssI = ssI + 1 } }
triSimUntil { (count (units ssFoeB)) >= 3 }

// pose: shooters combat line (up/crouch/prone like ssFight), targets standing
call { ssI = 0; while "ssI < 3" do { ssU = (units ssGrpA) select ssI; ssU setCaptive true; ssU setBehaviour "COMBAT"; ssU setDir 13; ssU disableAI "MOVE"; if (ssI == 1) then { ssU setUnitPos "MIDDLE" }; if (ssI == 2) then { ssU setUnitPos "DOWN" }; if (ssI == 0) then { ssU setUnitPos "UP" }; ssI = ssI + 1 } }
call { ssI = 0; while "ssI < 3" do { ssU = (units ssGrpB) select ssI; ssU setCaptive true; ssU setBehaviour "COMBAT"; ssU setDir 13; ssU disableAI "MOVE"; ssU setUnitPos "UP"; ssI = ssI + 1 } }
call { { _x setCaptive true; _x setBehaviour "COMBAT"; _x setUnitPos "UP"; _x setDir 193; _x disableAI "MOVE" } forEach (units ssFoeA) }
call { { _x setCaptive true; _x setBehaviour "COMBAT"; _x setUnitPos "UP"; _x setDir 193; _x disableAI "MOVE" } forEach (units ssFoeB) }
triSimFrames 60

// ammo before
ssA0 = (units ssGrpA select 0) ammo (primaryWeapon (units ssGrpA select 0))
ssA1 = (units ssGrpA select 1) ammo (primaryWeapon (units ssGrpA select 1))
ssA2 = (units ssGrpA select 2) ammo (primaryWeapon (units ssGrpA select 2))
ssB0 = (units ssGrpB select 0) ammo (primaryWeapon (units ssGrpB select 0))
ssB1 = (units ssGrpB select 1) ammo (primaryWeapon (units ssGrpB select 1))
ssB2 = (units ssGrpB select 2) ammo (primaryWeapon (units ssGrpB select 2))

// reveal + doFire, one foe per shooter
call { ssI = 0; while "ssI < 3" do { (units ssGrpA select 0) reveal (units ssFoeA select ssI); (units ssGrpB select 0) reveal (units ssFoeB select ssI); ssI = ssI + 1 } }
call { ssI = 0; while "ssI < 3" do { (units ssGrpA select ssI) doFire (units ssFoeA select ssI); (units ssGrpB select ssI) doFire (units ssFoeB select ssI); ssI = ssI + 1 } }

// camera: behind trio A on the shooter->target line, real-shoot composition
ssTgt = units ssGrpA select 1
ssAimH = 1.5
ssOff = [-1.6, 1.9, -7]
call ssAim
triSimFrames 10
triScreenshot "p01_f10"
triSimFrames 10
triScreenshot "p02_f20"
triSimFrames 20
triScreenshot "p03_f40"
triSimFrames 40
triScreenshot "p04_f80"

// profile view, perpendicular to the fire axis: weapon elevation readable
ssTgt = units ssGrpA select 1
ssAimH = 1.3
ssOff = [9.7, 1.4, -2.2]
call ssAim
triSimFrames 10
triScreenshot "p05_profile"
triSimFrames 50
triScreenshot "p06_profile_f140"

// trio B, same composition
ssTgt = units ssGrpB select 1
ssAimH = 1.5
ssOff = [-1.6, 1.9, -7]
call ssAim
triSimFrames 10
triScreenshot "p07_trioB"

// ammo after
ssC0 = (units ssGrpA select 0) ammo (primaryWeapon (units ssGrpA select 0))
ssC1 = (units ssGrpA select 1) ammo (primaryWeapon (units ssGrpA select 1))
ssC2 = (units ssGrpA select 2) ammo (primaryWeapon (units ssGrpA select 2))
ssD0 = (units ssGrpB select 0) ammo (primaryWeapon (units ssGrpB select 0))
ssD1 = (units ssGrpB select 1) ammo (primaryWeapon (units ssGrpB select 1))
ssD2 = (units ssGrpB select 2) ammo (primaryWeapon (units ssGrpB select 2))
ssAliveA = count (units ssFoeA)
ssAliveB = count (units ssFoeB)

// trio A must actually have fired
triAssertGe [(ssA0 - ssC0) + (ssA1 - ssC1) + (ssA2 - ssC2), 1]

// payload dump: per-man rounds fired + surviving targets (assert designed to fail)
ssOut = format ["A fired %1/%2/%3 foesLeft %4 | B fired %5/%6/%7 foesLeft %8", ssA0 - ssC0, ssA1 - ssC1, ssA2 - ssC2, ssAliveA, ssB0 - ssD0, ssB1 - ssD1, ssB2 - ssD2, ssAliveB]
triAssertEq [ssOut, "PROBE"]
triEndTest
