triSimUntil { not (isNil "GM_AS_READY") }
GM_AS_REMAINING = 100000
GM_SHK_CHANCE = -100
GM_COMP_ALIVE = [false]
GM_CIV_RADIUS = 0
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
removeAllWeapons player
player setCaptive true
player setPos [7500, 5700, 0]
asZone = gmZoneIndex "Village"
asG = [asZone] call GM_fnCivSpawnTown
asShooter = leader asG
asShooter setPos [7480, 5700, 0]
asShooter setDir 90
asShooter disableAI "MOVE"
asShooter stop true
asShooter allowDammage false
asShots = 0
asShooter addEventHandler ["fired", {asShots = asShots + 1}]
asTG = createGroup civilian
"Civilian2" createUnit [[7500,5700,0],asTG,"asTarget = this",0.5,"PRIVATE"]
player setPos [7600,5700,0]
asTarget disableAI "TARGET"
asTarget disableAI "AUTOTARGET"
asTarget disableAI "MOVE"
asTarget stop true
(group asTarget) setCombatMode "BLUE"
asTarget allowDammage false
triAssert [gmAssailantRegister [asShooter,"ROGUE",player,"Village"]]
asProbeTime = time
triSimUntil { time > asProbeTime + 10 }
logInfo format ["ASSAILANT_PROBE shots=%1 side=%2 behavior=%3 combat=%4 canFire=%5 knows=%6 weapons=%7 mags=%8", asShots, side asShooter, behaviour asShooter, combatMode asShooter, canFire asShooter, (asShooter knowsAbout asTarget), weapons asShooter, magazines asShooter]
triEndTest
