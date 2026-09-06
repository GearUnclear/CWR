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
asTarget = player
asTarget allowDammage false
triAssert [gmAssailantRegister [asShooter,"ROGUE",player,"Village"]]
triSimUntil { asShots > 0 }
triAssert [captive player]
triAssertEq [gmUndercoverStatus, 0]
triAssertEq [gmUndercoverWitnesses, 0]
triAssertGe [((group asShooter) knowsAbout asTarget), 0.1]
triEndTest
