triSimUntil { alive player }
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
leanWall = "UDLeanWall" createVehicle [7500,5700.55,0]
leanWall setPos [7500,5700.55,0]
leanWall setDir 45
leanSettle = time
triSimUntil { time > leanSettle + 1 }
getPos player
triLeanPose player
triFirePlayerWeapon
triSimUntil { (getDammage leanWall) > 0 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
leanWall = "UDLeanWall" createVehicle [7500,5700.55,0]
leanWall setPos [7500,5700.55,0]
leanWall setDir 60
leanSettle = time
triSimUntil { time > leanSettle + 1 }
getPos player
triLeanPose player
triFirePlayerWeapon
triSimUntil { (getDammage leanWall) > 0 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
leanWall = "UDLeanWall" createVehicle [7500,5700.55,0]
leanWall setPos [7500,5700.55,0]
leanWall setDir 80
leanSettle = time
triSimUntil { time > leanSettle + 1 }
getPos player
triLeanPose player
triFirePlayerWeapon
triSimUntil { (getDammage leanWall) > 0 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
leanWall = "UDLeanWall" createVehicle [7500,5700.55,0]
leanWall setPos [7500,5700.55,0]
leanWall setDir 100
leanSettle = time
triSimUntil { time > leanSettle + 1 }
getPos player
triLeanPose player
triFirePlayerWeapon
triSimUntil { (getDammage leanWall) > 0 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
leanWall = "UDLeanWall" createVehicle [7500,5700.55,0]
leanWall setPos [7500,5700.55,0]
leanWall setDir 120
leanSettle = time
triSimUntil { time > leanSettle + 1 }
getPos player
triLeanPose player
triFirePlayerWeapon
triSimUntil { (getDammage leanWall) > 0 }
deleteVehicle leanWall
player setPos [7500,5700,0]
player setDir 0
leanSettle = time
triSimUntil { time > leanSettle + 1 }
leanWall = "UDLeanWall" createVehicle [7500,5700.55,0]
leanWall setPos [7500,5700.55,0]
leanWall setDir 135
leanSettle = time
triSimUntil { time > leanSettle + 1 }
getPos player
triLeanPose player
triFirePlayerWeapon
triSimUntil { (getDammage leanWall) > 0 }
deleteVehicle leanWall
triEndTest
