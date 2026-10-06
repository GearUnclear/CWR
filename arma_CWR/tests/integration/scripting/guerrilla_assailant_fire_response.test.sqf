// The converted body's perceived side must follow its new group. Otherwise
// assailants can shoot soldiers while the soldiers still see harmless CIVs.
// Exercise both campaign war sides through normal perception and fire AI.
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
player setPos [7600, 5700, 0]
asZone = gmZoneIndex "Village"
asG = [asZone] call GM_fnCivSpawnTown
asTarget = leader asG
asTarget setPos [7500, 5700, 0]
asTarget disableAI "TARGET"
asTarget disableAI "AUTOTARGET"
asTarget disableAI "MOVE"
asTarget stop true
asTarget allowDammage false
triAssert [gmAssailantRegister [asTarget, "RESISTER", player, "Village"]]
(group asTarget) setCombatMode "BLUE"
triAssertEq [format ["%1", side asTarget], "WEST"]

// Occupier (EAST) sees the same civilian model as a live WEST enemy.
asMilitaryGroup = createGroup east
"SoldierEB" createUnit [[7480,5700,0],asMilitaryGroup,"asMilitary = this",0.5,"PRIVATE"]
asMilitary setDir 90
asMilitary disableAI "MOVE"
asMilitary allowDammage false
asMilitaryGroup setCombatMode "RED"
asMilitaryGroup setBehaviour "COMBAT"
asMilitaryShots = 0
asMilitary addEventHandler ["fired", {asMilitaryShots = asMilitaryShots + 1}]
triSimUntil { asMilitaryShots > 0 }
triAssertGe [(asMilitaryGroup knowsAbout asTarget), 0.1]
triAssertEq [gmAssailantClass asTarget, "RESISTER"]
deleteVehicle asMilitary
deleteGroup asMilitaryGroup

// Resistance (GUER) must independently acquire and respond as well.
asMilitaryGroup = createGroup resistance
"SoldierGB" createUnit [[7480,5700,0],asMilitaryGroup,"asMilitary = this",0.5,"PRIVATE"]
asMilitary setDir 90
asMilitary disableAI "MOVE"
asMilitary allowDammage false
asMilitaryGroup setCombatMode "RED"
asMilitaryGroup setBehaviour "COMBAT"
asMilitaryShots = 0
asMilitary addEventHandler ["fired", {asMilitaryShots = asMilitaryShots + 1}]
triSimUntil { asMilitaryShots > 0 }
triAssertGe [(asMilitaryGroup knowsAbout asTarget), 0.1]
triAssert [captive player]
triAssertEq [gmUndercoverStatus, 0]
triAssertEq [gmAssailantCount "ALL", 1]
triEndTest
