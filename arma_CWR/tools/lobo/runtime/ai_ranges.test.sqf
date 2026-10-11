// Controlled AI firing observations. A doFire order still uses native weapon selection.
// Actual distance and height are logged. Terrain/visibility remain part of each observation.
triSimUntil { alive player }
player allowDammage false
player setCaptive true
createCenter east
createCenter west
laBase = getPos player
laShooterPos = [(laBase select 0) - 100,(laBase select 1) + 70,0]
laWG = createGroup west
laEG = createGroup east
// LoBoM240_100m
"SoldierWB" createUnit [laShooterPos,laWG,"laShooter=this",1,"PRIVATE"]
triAssert [not isNull laShooter]
"SoldierEB" createUnit [[laShooterPos select 0,(laShooterPos select 1)+100,0],laEG,"laTarget=this",1,"PRIVATE"]
triAssert [not isNull laTarget]
removeAllWeapons laTarget
laTarget allowDammage false
laTarget setCaptive true
laTarget disableAI "MOVE"
laTarget setUnitPos "UP"
removeAllWeapons laShooter
laShooter addMagazine "LoBoM240Mag"
laShooter addWeapon "LoBoM240"
laShooter disableAI "MOVE"
laShooter setDir 0
laShooter setUnitPos "UP"
laShooter setBehaviour "COMBAT"
laShooter setCombatMode "RED"
triSimFrames 60
laShots=0
laShooter addEventHandler ["Fired",{laShots=laShots+1; logInfo format ["LOBO_AI_SHOT weapon=%1 projectile=%2 total=%3 time=%4",_this select 1,_this select 4,laShots,time]}]
laShooter reveal laTarget
laShooter doTarget laTarget
laShooter doFire laTarget
laWindowEnd=time+30
triSimUntil {time >= laWindowEnd}
triAssertGe [laShots,1]
logInfo format ["LOBO_AI case=LoBoM240_100m fired=%1 distance=%2 shooterASL=%3 targetASL=%4 remaining=%5",laShots,laShooter distance laTarget,getPosASL laShooter,getPosASL laTarget,laShooter ammo "LoBoM240"]
deleteVehicle laShooter
deleteVehicle laTarget
triSimFrames 5
// LoBoM240_300m
"SoldierWB" createUnit [laShooterPos,laWG,"laShooter=this",1,"PRIVATE"]
triAssert [not isNull laShooter]
"SoldierEB" createUnit [[laShooterPos select 0,(laShooterPos select 1)+300,0],laEG,"laTarget=this",1,"PRIVATE"]
triAssert [not isNull laTarget]
removeAllWeapons laTarget
laTarget allowDammage false
laTarget setCaptive true
laTarget disableAI "MOVE"
laTarget setUnitPos "UP"
removeAllWeapons laShooter
laShooter addMagazine "LoBoM240Mag"
laShooter addWeapon "LoBoM240"
laShooter disableAI "MOVE"
laShooter setDir 0
laShooter setUnitPos "UP"
laShooter setBehaviour "COMBAT"
laShooter setCombatMode "RED"
triSimFrames 60
laShots=0
laShooter addEventHandler ["Fired",{laShots=laShots+1; logInfo format ["LOBO_AI_SHOT weapon=%1 projectile=%2 total=%3 time=%4",_this select 1,_this select 4,laShots,time]}]
laShooter reveal laTarget
laShooter doTarget laTarget
laShooter doFire laTarget
laWindowEnd=time+30
triSimUntil {time >= laWindowEnd}
logInfo format ["LOBO_AI case=LoBoM240_300m fired=%1 distance=%2 shooterASL=%3 targetASL=%4 remaining=%5",laShots,laShooter distance laTarget,getPosASL laShooter,getPosASL laTarget,laShooter ammo "LoBoM240"]
deleteVehicle laShooter
deleteVehicle laTarget
triSimFrames 5
// LoBoM240_500m
"SoldierWB" createUnit [laShooterPos,laWG,"laShooter=this",1,"PRIVATE"]
triAssert [not isNull laShooter]
"SoldierEB" createUnit [[laShooterPos select 0,(laShooterPos select 1)+500,0],laEG,"laTarget=this",1,"PRIVATE"]
triAssert [not isNull laTarget]
removeAllWeapons laTarget
laTarget allowDammage false
laTarget setCaptive true
laTarget disableAI "MOVE"
laTarget setUnitPos "UP"
removeAllWeapons laShooter
laShooter addMagazine "LoBoM240Mag"
laShooter addWeapon "LoBoM240"
laShooter disableAI "MOVE"
laShooter setDir 0
laShooter setUnitPos "UP"
laShooter setBehaviour "COMBAT"
laShooter setCombatMode "RED"
triSimFrames 60
laShots=0
laShooter addEventHandler ["Fired",{laShots=laShots+1; logInfo format ["LOBO_AI_SHOT weapon=%1 projectile=%2 total=%3 time=%4",_this select 1,_this select 4,laShots,time]}]
laShooter reveal laTarget
laShooter doTarget laTarget
laShooter doFire laTarget
laWindowEnd=time+30
triSimUntil {time >= laWindowEnd}
logInfo format ["LOBO_AI case=LoBoM240_500m fired=%1 distance=%2 shooterASL=%3 targetASL=%4 remaining=%5",laShots,laShooter distance laTarget,getPosASL laShooter,getPosASL laTarget,laShooter ammo "LoBoM240"]
deleteVehicle laShooter
deleteVehicle laTarget
triSimFrames 5
// LoBoM249_100m
"SoldierWB" createUnit [laShooterPos,laWG,"laShooter=this",1,"PRIVATE"]
triAssert [not isNull laShooter]
"SoldierEB" createUnit [[laShooterPos select 0,(laShooterPos select 1)+100,0],laEG,"laTarget=this",1,"PRIVATE"]
triAssert [not isNull laTarget]
removeAllWeapons laTarget
laTarget allowDammage false
laTarget setCaptive true
laTarget disableAI "MOVE"
laTarget setUnitPos "UP"
removeAllWeapons laShooter
laShooter addMagazine "LoBoM249Mag"
laShooter addWeapon "LoBoM249"
laShooter disableAI "MOVE"
laShooter setDir 0
laShooter setUnitPos "UP"
laShooter setBehaviour "COMBAT"
laShooter setCombatMode "RED"
triSimFrames 60
laShots=0
laShooter addEventHandler ["Fired",{laShots=laShots+1; logInfo format ["LOBO_AI_SHOT weapon=%1 projectile=%2 total=%3 time=%4",_this select 1,_this select 4,laShots,time]}]
laShooter reveal laTarget
laShooter doTarget laTarget
laShooter doFire laTarget
laWindowEnd=time+30
triSimUntil {time >= laWindowEnd}
triAssertGe [laShots,1]
logInfo format ["LOBO_AI case=LoBoM249_100m fired=%1 distance=%2 shooterASL=%3 targetASL=%4 remaining=%5",laShots,laShooter distance laTarget,getPosASL laShooter,getPosASL laTarget,laShooter ammo "LoBoM249"]
deleteVehicle laShooter
deleteVehicle laTarget
triSimFrames 5
// LoBoM249_300m
"SoldierWB" createUnit [laShooterPos,laWG,"laShooter=this",1,"PRIVATE"]
triAssert [not isNull laShooter]
"SoldierEB" createUnit [[laShooterPos select 0,(laShooterPos select 1)+300,0],laEG,"laTarget=this",1,"PRIVATE"]
triAssert [not isNull laTarget]
removeAllWeapons laTarget
laTarget allowDammage false
laTarget setCaptive true
laTarget disableAI "MOVE"
laTarget setUnitPos "UP"
removeAllWeapons laShooter
laShooter addMagazine "LoBoM249Mag"
laShooter addWeapon "LoBoM249"
laShooter disableAI "MOVE"
laShooter setDir 0
laShooter setUnitPos "UP"
laShooter setBehaviour "COMBAT"
laShooter setCombatMode "RED"
triSimFrames 60
laShots=0
laShooter addEventHandler ["Fired",{laShots=laShots+1; logInfo format ["LOBO_AI_SHOT weapon=%1 projectile=%2 total=%3 time=%4",_this select 1,_this select 4,laShots,time]}]
laShooter reveal laTarget
laShooter doTarget laTarget
laShooter doFire laTarget
laWindowEnd=time+30
triSimUntil {time >= laWindowEnd}
logInfo format ["LOBO_AI case=LoBoM249_300m fired=%1 distance=%2 shooterASL=%3 targetASL=%4 remaining=%5",laShots,laShooter distance laTarget,getPosASL laShooter,getPosASL laTarget,laShooter ammo "LoBoM249"]
deleteVehicle laShooter
deleteVehicle laTarget
triSimFrames 5
// LoBoNegev_300m
"SoldierWB" createUnit [laShooterPos,laWG,"laShooter=this",1,"PRIVATE"]
triAssert [not isNull laShooter]
"SoldierEB" createUnit [[laShooterPos select 0,(laShooterPos select 1)+300,0],laEG,"laTarget=this",1,"PRIVATE"]
triAssert [not isNull laTarget]
removeAllWeapons laTarget
laTarget allowDammage false
laTarget setCaptive true
laTarget disableAI "MOVE"
laTarget setUnitPos "UP"
removeAllWeapons laShooter
laShooter addMagazine "LoBoNegevMag"
laShooter addWeapon "LoBoNegev"
laShooter disableAI "MOVE"
laShooter setDir 0
laShooter setUnitPos "UP"
laShooter setBehaviour "COMBAT"
laShooter setCombatMode "RED"
triSimFrames 60
laShots=0
laShooter addEventHandler ["Fired",{laShots=laShots+1; logInfo format ["LOBO_AI_SHOT weapon=%1 projectile=%2 total=%3 time=%4",_this select 1,_this select 4,laShots,time]}]
laShooter reveal laTarget
laShooter doTarget laTarget
laShooter doFire laTarget
laWindowEnd=time+30
triSimUntil {time >= laWindowEnd}
logInfo format ["LOBO_AI case=LoBoNegev_300m fired=%1 distance=%2 shooterASL=%3 targetASL=%4 remaining=%5",laShots,laShooter distance laTarget,getPosASL laShooter,getPosASL laTarget,laShooter ammo "LoBoNegev"]
deleteVehicle laShooter
deleteVehicle laTarget
triSimFrames 5
// LoBoNegev_Commando_SD_100m
"SoldierWB" createUnit [laShooterPos,laWG,"laShooter=this",1,"PRIVATE"]
triAssert [not isNull laShooter]
"SoldierEB" createUnit [[laShooterPos select 0,(laShooterPos select 1)+100,0],laEG,"laTarget=this",1,"PRIVATE"]
triAssert [not isNull laTarget]
removeAllWeapons laTarget
laTarget allowDammage false
laTarget setCaptive true
laTarget disableAI "MOVE"
laTarget setUnitPos "UP"
removeAllWeapons laShooter
laShooter addMagazine "LoBoNegevSDMag"
laShooter addWeapon "LoBoNegev_Commando_SD"
laShooter disableAI "MOVE"
laShooter setDir 0
laShooter setUnitPos "UP"
laShooter setBehaviour "COMBAT"
laShooter setCombatMode "RED"
triSimFrames 60
laShots=0
laShooter addEventHandler ["Fired",{laShots=laShots+1; logInfo format ["LOBO_AI_SHOT weapon=%1 projectile=%2 total=%3 time=%4",_this select 1,_this select 4,laShots,time]}]
laShooter reveal laTarget
laShooter doTarget laTarget
laShooter doFire laTarget
laWindowEnd=time+30
triSimUntil {time >= laWindowEnd}
triAssertGe [laShots,1]
logInfo format ["LOBO_AI case=LoBoNegev_Commando_SD_100m fired=%1 distance=%2 shooterASL=%3 targetASL=%4 remaining=%5",laShots,laShooter distance laTarget,getPosASL laShooter,getPosASL laTarget,laShooter ammo "LoBoNegev_Commando_SD"]
deleteVehicle laShooter
deleteVehicle laTarget
triSimFrames 5
// LoBoNegev_Commando_SD_300m
"SoldierWB" createUnit [laShooterPos,laWG,"laShooter=this",1,"PRIVATE"]
triAssert [not isNull laShooter]
"SoldierEB" createUnit [[laShooterPos select 0,(laShooterPos select 1)+300,0],laEG,"laTarget=this",1,"PRIVATE"]
triAssert [not isNull laTarget]
removeAllWeapons laTarget
laTarget allowDammage false
laTarget setCaptive true
laTarget disableAI "MOVE"
laTarget setUnitPos "UP"
removeAllWeapons laShooter
laShooter addMagazine "LoBoNegevSDMag"
laShooter addWeapon "LoBoNegev_Commando_SD"
laShooter disableAI "MOVE"
laShooter setDir 0
laShooter setUnitPos "UP"
laShooter setBehaviour "COMBAT"
laShooter setCombatMode "RED"
triSimFrames 60
laShots=0
laShooter addEventHandler ["Fired",{laShots=laShots+1; logInfo format ["LOBO_AI_SHOT weapon=%1 projectile=%2 total=%3 time=%4",_this select 1,_this select 4,laShots,time]}]
laShooter reveal laTarget
laShooter doTarget laTarget
laShooter doFire laTarget
laWindowEnd=time+30
triSimUntil {time >= laWindowEnd}
logInfo format ["LOBO_AI case=LoBoNegev_Commando_SD_300m fired=%1 distance=%2 shooterASL=%3 targetASL=%4 remaining=%5",laShots,laShooter distance laTarget,getPosASL laShooter,getPosASL laTarget,laShooter ammo "LoBoNegev_Commando_SD"]
deleteVehicle laShooter
deleteVehicle laTarget
triSimFrames 5
triEndTest
