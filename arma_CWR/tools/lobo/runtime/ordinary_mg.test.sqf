// Ordinary projectile collision and nearby-infantry splash check.
triSimUntil {alive player}
player allowDammage false
player setCaptive true
createCenter east
laEG=createGroup east
laBase=getPos player
laPos=[(laBase select 0)-100,(laBase select 1)+100,0]
// LoBo_M240Coax
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="LoBo_M240Coax" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=LoBo_M240Coax directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
// LoBo_M240C
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="LoBo_M240C" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=LoBo_M240C directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
// LoBo_50cal
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="LoBo_50cal" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=LoBo_50cal directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
// loBo_DTMCoax
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="loBo_DTMCoax" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=loBo_DTMCoax directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
// loBo_PKCoax
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="loBo_PKCoax" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=loBo_PKCoax directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
// loBo_DShK
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="loBo_DShK" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=loBo_DShK directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
// loBo_KPVT
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="loBo_KPVT" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=loBo_KPVT directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
// loBo_DShK_Ter
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="loBo_DShK_Ter" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=loBo_DShK_Ter directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
// LoBo_zpu4Bullet
"SoldierEB" createUnit [laPos,laEG,"laDirect=this",1,"PRIVATE"]
"SoldierEB" createUnit [[(laPos select 0)+2,laPos select 1,0],laEG,"laNear=this",1,"PRIVATE"]
triAssert [not isNull laDirect]
triAssert [not isNull laNear]
laDirect setPos laPos
laNear setPos [(laPos select 0)+2,laPos select 1,0]
removeAllWeapons laDirect
removeAllWeapons laNear
laDirect disableAI "MOVE"
laNear disableAI "MOVE"
laDirect setUnitPos "UP"
laNear setUnitPos "UP"
triSimFrames 15
laBeforeDistance=laDirect distance laNear
triAssertLe [abs (laBeforeDistance-2),0.1]
laP=getPos laDirect
laBullet="LoBo_zpu4Bullet" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laBullet]
laBullet setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_ORDINARY_MG ammo=LoBo_zpu4Bullet directDamage=%1 nearbyDamage=%2 finalSeparation=%3 beforeSeparation=%4",getDammage laDirect,getDammage laNear,laDirect distance laNear,laBeforeDistance]
triAssertGt [getDammage laDirect,0.01]
triAssertEq [getDammage laNear,0]
deleteVehicle laDirect
deleteVehicle laNear
triSimFrames 5
triEndTest
