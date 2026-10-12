// ============================================================================
//  LoBo #4 runtime smoke probe (repair 2026-10-11.3, installed @LoBo only).
//
//  One Mk-20 Rockeye from each aircraft whose fired script used a missing
//  rockeye.sqs path before .3: A-4R (firemissiles_A2.sqs), F-4 WW
//  (firemissiles_WW.sqs) and IL-28 AS (fire_mk20.sqs). Each now runs
//  LoBo_airammo Scripts\cbu\rockeye.sqs. Passes when every drop opens a
//  LoBo_mk20Deployed dispenser and LoBoMk20Bomblet objects exist. The
//  aircraft is held still at 150 m, invulnerable, and parked on the runway
//  once the fired script has placed the bomb, so this checks the script path,
//  not ballistics, spread or damage.
// ============================================================================
#include "lobo_cluster_observe.sqf"
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
player allowDammage false
player setCaptive true
lcCenter = [8000, 9900, 0]
lcDrop = [8000, 9900, 150]
lcPark = [8000, 9520, 0]
lcBomb = "LoBo_mk20Bomb"
lcDeployed = "LoBo_mk20Deployed"
lcBomblet = "LoBoMk20Bomblet"

// Control: the observer must see a bomblet created directly by class name.
lcCtl = "LoBoMk20Bomblet" createVehicle [8000, 9900, 300]
triAssert [not isNull lcCtl]
triAssertEq [typeOf lcCtl, "LoBoMk20Bomblet"]
triAssert [lcCtl in (nearestObjects [lcCenter, [], 400])]
logInfo format ["LOBO_R3_CONTROL class=%1 observed=%2", typeOf lcCtl, lcCtl in (nearestObjects [lcCenter, [], 400])]
deleteVehicle lcCtl
triSimFrames 5

// --- A-4R: LoBo_Mk20Rail4 through firemissiles_A2.sqs ---
lcWeapon = "LoBo_Mk20Rail4"
lcPlane = "LoBo_A4R" createVehicle lcPark
triAssertEq [typeOf lcPlane, "LoBo_A4R"]
lcPlane allowDammage false
player moveInDriver lcPlane
triAssertEq [vehicle player, lcPlane]
triSimFrames 30
triAssertEq [lcPlane ammo lcWeapon, 4]
lcFired = []
lcPlane addEventHandler ["fired", {lcFired = lcFired + [[_this select 1, _this select 4]]}]
call lcReset
lcPlane setPos lcDrop
lcPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
lcPlane setVelocity [0,0,0]
lcPlane selectWeapon lcWeapon
lcPlane fire lcWeapon
triSimUntil { (lcPlane ammo lcWeapon) == 3 }
triSimFrames 8
lcPlane setPos lcPark
lcPlane setVelocity [0,0,0]
triSimUntil { call lcObserve }
logInfo format ["LOBO_R3_CLUSTER case=a4r plane=%1 fired=%2 bombSeen=%3 bombMinZ=%4 deployedMax=%5 bombletsSeen=%6 bombletsMaxLive=%7 polls=%8 elapsed=%9", typeOf lcPlane, lcFired, lcBombSeen, lcBombMinZ, lcMaxDeployed, count lcSeen, lcMaxLive, lcPolls, time - lcT0]
triAssertEq [count lcFired, 1]
triAssertEq [(lcFired select 0) select 1, lcBomb]
triAssertGt [lcMaxDeployed, 0]
triAssertGt [count lcSeen, 0]
player action ["GETOUT", lcPlane]
triSimUntil { vehicle player == player }
deleteVehicle lcPlane
triSimFrames 3

// --- F-4 WW: LoBo_Mk20Rail6 through firemissiles_WW.sqs ---
// hasGunner=1: the weapon systems officer seat owns the stores, so the player sits there.
lcWeapon = "LoBo_Mk20Rail6"
lcPlane = "LoBo_F4_WW_IAF" createVehicle lcPark
triAssertEq [typeOf lcPlane, "LoBo_F4_WW_IAF"]
lcPlane allowDammage false
player moveInGunner lcPlane
triAssertEq [vehicle player, lcPlane]
triSimFrames 30
triAssertEq [lcPlane ammo lcWeapon, 6]
lcFired = []
lcPlane addEventHandler ["fired", {lcFired = lcFired + [[_this select 1, _this select 4]]}]
call lcReset
lcPlane setPos lcDrop
lcPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
lcPlane setVelocity [0,0,0]
lcPlane selectWeapon lcWeapon
lcPlane fire lcWeapon
triSimUntil { (lcPlane ammo lcWeapon) == 5 }
triSimFrames 8
lcPlane setPos lcPark
lcPlane setVelocity [0,0,0]
triSimUntil { call lcObserve }
logInfo format ["LOBO_R3_CLUSTER case=f4ww plane=%1 fired=%2 bombSeen=%3 bombMinZ=%4 deployedMax=%5 bombletsSeen=%6 bombletsMaxLive=%7 polls=%8 elapsed=%9", typeOf lcPlane, lcFired, lcBombSeen, lcBombMinZ, lcMaxDeployed, count lcSeen, lcMaxLive, lcPolls, time - lcT0]
triAssertEq [count lcFired, 1]
triAssertEq [(lcFired select 0) select 1, lcBomb]
triAssertGt [lcMaxDeployed, 0]
triAssertGt [count lcSeen, 0]
player action ["GETOUT", lcPlane]
triSimUntil { vehicle player == player }
deleteVehicle lcPlane
triSimFrames 3

// --- IL-28 AS: LoBo_Mk20Rail10 through fire_mk20.sqs ---
// hasGunner=1: the bombardier seat owns the stores, so the player sits there.
// The installed tailgun.sqs runs 0.1 s after Init. It takes its turret gunner
// from the pilot's group (a crewed IL-28 without a pilot raises a script error
// there) and ejects the crew of a pilotless IL-28 above 20 m, so an AI pilot
// boards in the same frame as the spawn, before the player takes the seat.
lcWeapon = "LoBo_Mk20Rail10"
call {lcPlane = "LoBo_IL28_AS_egy" createVehicle lcPark; lcGrp = createGroup west; "LoBo_Egypt_Pilot" createUnit [lcPark, lcGrp, "lcPilot = this", 0.5, "PRIVATE"]; lcPilot moveInDriver lcPlane; player moveInGunner lcPlane}
triAssertEq [typeOf lcPlane, "LoBo_IL28_AS_egy"]
triAssertEq [driver lcPlane, lcPilot]
triAssertEq [vehicle player, lcPlane]
lcPlane allowDammage false
lcPilot allowDammage false
lcPilot setCaptive true
triSimFrames 30
logInfo format ["LOBO_R3_IL28_CREW driver=%1 gunner=%2 crew=%3 turrets=%4", typeOf (driver lcPlane), gunner lcPlane == player, count (crew lcPlane), count (nearestObjects [lcPlane, ["LoBo_rearturret"], 60])]
triAssertEq [lcPlane ammo lcWeapon, 10]
lcFired = []
lcPlane addEventHandler ["fired", {lcFired = lcFired + [[_this select 1, _this select 4]]}]
call lcReset
lcPlane setPos lcDrop
lcPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
lcPlane setVelocity [0,0,0]
lcPlane selectWeapon lcWeapon
lcPlane fire lcWeapon
triSimUntil { (lcPlane ammo lcWeapon) == 9 }
triSimFrames 8
lcPlane setPos lcPark
lcPlane setVelocity [0,0,0]
triSimUntil { call lcObserve }
logInfo format ["LOBO_R3_CLUSTER case=il28as plane=%1 fired=%2 bombSeen=%3 bombMinZ=%4 deployedMax=%5 bombletsSeen=%6 bombletsMaxLive=%7 polls=%8 elapsed=%9", typeOf lcPlane, lcFired, lcBombSeen, lcBombMinZ, lcMaxDeployed, count lcSeen, lcMaxLive, lcPolls, time - lcT0]
triAssertEq [count lcFired, 1]
triAssertEq [(lcFired select 0) select 1, lcBomb]
triAssertGt [lcMaxDeployed, 0]
triAssertGt [count lcSeen, 0]
player action ["GETOUT", lcPlane]
triSimUntil { vehicle player == player }
deleteVehicle lcPlane
triSimFrames 3
triEndTest
