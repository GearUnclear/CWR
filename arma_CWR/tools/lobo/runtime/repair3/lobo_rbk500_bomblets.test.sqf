// ============================================================================
//  LoBo #5 runtime smoke probe (repair 2026-10-11.3, installed @LoBo only).
//
//  The Syrian MiG-21 CAS1 releases one RBK-500 through its installed fired
//  handler (LoBoMig scripts\syria\firemissiles_CAS1.sqs, which runs
//  LoBo_airammo Scripts\cbu\rbk500.sqs). Before .3 that script created the
//  undefined class LoBoRBKBomblet, so the bomb vanished at 50 m with no
//  effect. Passes when the dispenser opens and LoBoRBK500Bomblet objects exist.
//  The aircraft is held still at 150 m, invulnerable, and parked on the
//  runway once the fired script has placed the bomb, so this checks the
//  script path, not ballistics, spread or damage.
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

// Control: the observer must see a bomblet created directly by class name.
lcCtl = "LoBoRBK500Bomblet" createVehicle [8000, 9900, 300]
triAssert [not isNull lcCtl]
triAssertEq [typeOf lcCtl, "LoBoRBK500Bomblet"]
triAssert [lcCtl in (nearestObjects [lcCenter, [], 400])]
logInfo format ["LOBO_R3_CONTROL class=%1 observed=%2", typeOf lcCtl, lcCtl in (nearestObjects [lcCenter, [], 400])]
deleteVehicle lcCtl
triSimFrames 5

lcBomb = "LoBo_RBK500Bomb"
lcDeployed = "LoBo_RBK500Deployed"
lcBomblet = "LoBoRBK500Bomblet"
lcWeapon = "LoBo_RBK500Rail2"
lcPlane = "LoBo_mig21bis_Syria_CAS1" createVehicle lcPark
triAssertEq [typeOf lcPlane, "LoBo_mig21bis_Syria_CAS1"]
lcPlane allowDammage false
player moveInDriver lcPlane
triAssertEq [vehicle player, lcPlane]
triSimFrames 30
triAssertEq [lcPlane ammo lcWeapon, 2]
lcFired = []
lcPlane addEventHandler ["fired", {lcFired = lcFired + [[_this select 1, _this select 4]]}]
call lcReset
lcPlane setPos lcDrop
lcPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
lcPlane setVelocity [0,0,0]
lcPlane selectWeapon lcWeapon
lcPlane fire lcWeapon
triSimUntil { (lcPlane ammo lcWeapon) == 1 }
// The fired script positions the bomb from the aircraft; then park it away.
triSimFrames 8
lcPlane setPos lcPark
lcPlane setVelocity [0,0,0]
triSimUntil { call lcObserve }
logInfo format ["LOBO_R3_CLUSTER case=rbk500 plane=%1 fired=%2 bombSeen=%3 bombMinZ=%4 deployedMax=%5 bombletsSeen=%6 bombletsMaxLive=%7 polls=%8 elapsed=%9", typeOf lcPlane, lcFired, lcBombSeen, lcBombMinZ, lcMaxDeployed, count lcSeen, lcMaxLive, lcPolls, time - lcT0]
triAssertEq [count lcFired, 1]
triAssertEq [(lcFired select 0) select 1, lcBomb]
triAssertGt [lcMaxDeployed, 0]
triAssertGt [count lcSeen, 0]
player action ["GETOUT", lcPlane]
triSimUntil { vehicle player == player }
deleteVehicle lcPlane
triSimFrames 3
triEndTest
