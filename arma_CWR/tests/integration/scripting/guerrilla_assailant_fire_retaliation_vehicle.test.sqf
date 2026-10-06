// Personal retaliation follows the extorter into a civilian car. Perception
// targets the occupied vehicle, while the assailant record stores the person.
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
asShooter allowDammage false
asShots = 0
asShooter addEventHandler ["fired", {asShots = asShots + 1}]
asCar = "Skoda" createVehicle [7500, 5700, 0]
asCar allowDammage false
player moveInDriver asCar
triAssertEq [vehicle player, asCar]
triAssert [captive player]
triAssert [gmAssailantRegister [asShooter, "RESISTER", player, "Village"]]
triAssertEq [gmAssailantClass asShooter, "RESISTER"]
triSimUntil { asShots > 0 }
triAssertEq [vehicle player, asCar]
triAssert [captive player]
triAssertEq [gmUndercoverStatus, 0]
triAssertGe [((group asShooter) knowsAbout asCar), 0.1]
triAssert [gmAssailantRemove asShooter]

// A dead extorter's lingering passenger brain must not make an innocent
// covered driver hostile. Use a fresh observer with no remembered car target.
asExtorterGroup = createGroup GM_CIV_SIDE
(GM_CIV_CLASSES select 0) createUnit [[7500,5700,0],asExtorterGroup,"asExtorter = this",0.5,"PRIVATE"]
asExtorter moveInCargo asCar
triAssertEq [vehicle asExtorter, asCar]
asG = [asZone] call GM_fnCivSpawnTown
asShooter = leader asG
asShooter setPos [7480, 5700, 0]
asShooter setDir 90
asShooter disableAI "MOVE"
asShooter disableAI "TARGET"
asShooter disableAI "AUTOTARGET"
asShooter allowDammage false
asShots = 0
asShooter addEventHandler ["fired", {asShots = asShots + 1}]
triAssert [gmAssailantRegister [asShooter, "RESISTER", asExtorter, "Village"]]
asExtorter setDammage 1
triAssert [not (alive asExtorter)]
triAssertEq [vehicle asExtorter, asCar]
asShooter enableAI "TARGET"
asShooter enableAI "AUTOTARGET"
asNoFireStart = time
triSimUntil { time > asNoFireStart + 10 }
triAssertGe [((group asShooter) knowsAbout asCar), 0.1]
triAssertEq [asShots, 0]
triAssert [captive player]
triAssertEq [vehicle player, asCar]
triEndTest
