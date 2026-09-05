// ============================================================================
//  Guerrilla Mode ambient road traffic - civilian DANGER RESPONSE (the first
//  rehearsal of the 2026-09-01 danger work, issue #55).
//    A civ car on the move hears a rifle shot from the roadside: the
//    frozen-core FireWeaponEffects hook (GTrafficDangerArmed) feeds the
//    danger ring, the next traffic pass rolls a reaction for the car inside
//    the band (cower / U-turn home / rush past / bail-and-run) and fires the
//    "panicked" event [veh, kind, reaction].
//  Asserted: THAT a reaction happens for THIS car within the pass window and
//  that it is one of the four named reactions - not which one (the roll is
//  random, and the source-selection rules are the open "panic priority"
//  item; this fence must survive that rework).
//  Staging: force-spawn from the Village, let the car drive off, then put the
//  ARMED player 30 m off its flank, slightly ahead, facing AWAY (outside the
//  25 m commandeer radius, no weapon aimed at the car - the commandeer must
//  not win this one), and force-fire one round (player_fire_headless proves
//  the force-fire path in this fixture).  Rifle audibleFire reads as
//  severity ~1 -> a 200 m reaction band; the car is well inside it when the
//  pass runs.
// ============================================================================

triSimUntil { GM_LIB_READY }

gtPanicked = []
gmTrafficOnEvent ["panicked", {gtPanicked = gtPanicked + [_this]}]

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5

gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]
gtDrv = driver gtCar
triAssertEq [(format ["%1", isNull gtDrv]), "false"]

// -- it drives (the reaction tier only arms for a live, moving leg) -------------
gtP0 = getPos gtCar
triSimUntil { ((getPos gtCar) distance gtP0) >= 15 }

// -- the roadside gunman: 40 m ahead along the heading, 30 m off the flank,
//    facing away from the road -------------------------------------------------
gtD = getDir gtCar
gtCp = getPos gtCar
gtAx = (gtCp select 0) + (40 * (sin gtD)) + (30 * (sin (gtD + 90)))
gtAy = (gtCp select 1) + (40 * (cos gtD)) + (30 * (cos (gtD + 90)))
player setPos [gtAx, gtAy, 0]
player setDir (gtD + 90)
triSimFrames 3
// nowhere near the commandeer trigger: still a live civ entry
triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "true"]

guWeap = primaryWeapon player
triAssertNe [guWeap, ""]
guAmmo0 = player ammo guWeap
triAssertGt [guAmmo0, 0]
player fire guWeap
triSimUntil { (player ammo guWeap) < guAmmo0 }

// -- the next pass: the driver reacts to THIS shot ------------------------------
triSimUntil { count gtPanicked >= 1 }
triAssertEq [(format ["%1", ((gtPanicked select 0) select 0) == gtCar]), "true"]
triAssertEq [((gtPanicked select 0) select 1), "civ"]
gtReact = (gtPanicked select 0) select 2
triAssertEq [(format ["%1", (gtReact == "cower") or (gtReact == "uturn") or (gtReact == "rush") or (gtReact == "bail")]), "true"]

// -- the reaction is consistent with the registry: a bail released the hull
//    (driver on foot, car gone from the live table); every other reaction
//    keeps the car a live civ entry with its driver seated ----------------------
triSimFrames 5
? gtReact == "bail" : triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "false"]
? gtReact == "bail" : triAssertEq [(format ["%1", (vehicle gtDrv) == gtDrv]), "true"]
? gtReact != "bail" : triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "true"]
? gtReact != "bail" : triAssertEq [(format ["%1", (driver gtCar) == gtDrv]), "true"]

triEndTest
