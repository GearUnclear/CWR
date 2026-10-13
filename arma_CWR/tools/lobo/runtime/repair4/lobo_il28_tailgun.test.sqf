// ============================================================================
//  LoBo #13 runtime probe (repair 2026-10-11.4, installed @LoBo only).
//
//  LoBoIL28 script\tailgun.sqs runs 0.1 s after an IL-28 spawns. Before .4 a
//  crew without a pilot made it select index -1 from an empty group ("Zero
//  divisor"), which aborts a run under --autotest. Cases:
//    C1  empty IL-28: unmanned turret, no error.
//    G   player in the gunner seat, no pilot, on the ground: unmanned turret.
//    C2  AI pilot first (group r4East): the turret gets a LoBo_Egypt_Pilot
//        gunner in the pilot's group.
//    A   player gunner, no pilot, at 400 m: the authored bailout ejects him.
//    C3  C2 lifted to 400 m, pilot killed: the turret gunner bails out into a
//        ParachuteEast.
//  Each spawn and its seating run in one evaluation (a call block), so the
//  script sees the crew as placed. The log is checked afterwards for
//  "Zero divisor" and tailgun.sqs script errors.
// ============================================================================
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
showCinemaBorder false
player allowDammage false
player setCaptive true
tgTurretOf = {_t = objNull; {if (isNull _t) then {_t = _x}} forEach (nearestObjects [getPos _this, ["LoBo_rearturret"], 60]); _t}

// --- C1: empty IL-28 ---
tgPlane = "LoBo_IL28_egy" createVehicle [7600, 9620, 0]
triAssertEq [typeOf tgPlane, "LoBo_IL28_egy"]
tgT = time
triSimUntil { time > tgT + 3 }
tgTurret = tgPlane call tgTurretOf
logInfo format ["LOBO_R4_TG case=C1 turret=%1 gunnerNull=%2 crew=%3", typeOf tgTurret, isNull gunner tgTurret, count crew tgPlane]
triAssert [not isNull tgTurret]
triAssertEq [isNull gunner tgTurret, true]

// --- G: player gunner, no pilot, ground ---
call {tgPlane = "LoBo_IL28_egy" createVehicle [8000, 9620, 0]; player moveInGunner tgPlane; tgT = time}
triAssertEq [typeOf tgPlane, "LoBo_IL28_egy"]
triSimUntil { time > tgT + 5 }
tgTurret = tgPlane call tgTurretOf
logInfo format ["LOBO_R4_TG case=G inPlane=%1 isGunner=%2 driverNull=%3 turret=%4 gunnerNull=%5 alt=%6", vehicle player == tgPlane, gunner tgPlane == player, isNull driver tgPlane, typeOf tgTurret, isNull gunner tgTurret, getPos tgPlane select 2]
triAssertEq [vehicle player == tgPlane, true]
triAssert [not isNull tgTurret]
triAssertEq [isNull gunner tgTurret, true]
triAssertLt [getPos tgPlane select 2, 20]
tgP = getPosASL tgPlane
triSetView [(tgP select 0) - 9, (tgP select 2) + 3.5, (tgP select 1) - 16, 9, -2, 10]
triSimFrames 5
triScreenshot "g_player_gunner_no_pilot"
triClearView

// --- C2: AI pilot first, in group r4East ---
call {tgPlane = "LoBo_IL28_egy" createVehicle [8400, 9620, 0]; "LoBo_Egypt_Pilot" createUnit [[8410, 9620, 0], group r4East, "", 0.6, "PRIVATE"]; tgPilot = (units group r4East) select ((count units group r4East) - 1); tgPilot assignAsDriver tgPlane; tgPilot moveInDriver tgPlane; tgT = time}
triAssertEq [typeOf tgPilot, "LoBo_Egypt_Pilot"]
triAssertEq [driver tgPlane == tgPilot, true]
triSimUntil { time > tgT + 3 }
tgTurret = tgPlane call tgTurretOf
tgTG = gunner tgTurret
logInfo format ["LOBO_R4_TG case=C2 turret=%1 gunner=%2 alive=%3 sameGroup=%4 pilotInDriver=%5", typeOf tgTurret, typeOf tgTG, alive tgTG, group tgTG == group tgPilot, driver tgPlane == tgPilot]
triAssert [not isNull tgTurret]
triAssertEq [alive tgTG, true]
triAssertEq [typeOf tgTG, "LoBo_Egypt_Pilot"]
triAssertEq [group tgTG == group tgPilot, true]

// --- C3: C2 at 400 m, pilot killed: turret gunner bails out ---
call {tgPlane setPos [8400, 9620, 400]; tgPilot setDammage 1; tgT = time}
triSimUntil { (vehicle tgTG != tgTurret) || (time > tgT + 5) }
logInfo format ["LOBO_R4_TG case=C3 pilotAlive=%1 tgVehicle=%2 tgAlive=%3 planeAlt=%4 dt=%5", alive tgPilot, typeOf vehicle tgTG, alive tgTG, getPos tgPlane select 2, time - tgT]
triAssertEq [alive tgPilot, false]
triAssertEq [typeOf vehicle tgTG, "ParachuteEast"]

// --- A: player gunner, no pilot, 400 m: authored bailout ejects the gunner ---
call {player action ["GETOUT", vehicle player]}
triSimUntil { vehicle player == player }
call {tgPlane = "LoBo_IL28_egy" createVehicle [8800, 9620, 0]; player moveInGunner tgPlane; tgPlane setPos [8800, 9620, 400]; tgT = time}
triAssertEq [typeOf tgPlane, "LoBo_IL28_egy"]
triSimUntil { (vehicle player != tgPlane) || (time > tgT + 5) }
tgTurret = tgPlane call tgTurretOf
logInfo format ["LOBO_R4_TG case=A inPlane=%1 playerVehicle=%2 turret=%3 gunnerNull=%4 planeAlt=%5 dt=%6", vehicle player == tgPlane, typeOf vehicle player, typeOf tgTurret, isNull gunner tgTurret, getPos tgPlane select 2, time - tgT]
triAssertEq [vehicle player != tgPlane, true]
triAssert [not isNull tgTurret]
triAssertEq [isNull gunner tgTurret, true]
triSimFrames 30
logInfo "LOBO_R4_TG_DONE"
triEndTest
