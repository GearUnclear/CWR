// Keep the incident clock paused by standing outside the 300 m town radius.
// Native and script managers still run; the source civilian group retains its
// third member, so the save must preserve three distinct group references.
triSimUntil { not (isNil "GM_AS_READY") }
GM_AS_REMAINING = 1379
GM_SHK_CHANCE = -100
GM_COMP_ALIVE = [false]
GM_CIV_RADIUS = 0
GM_CIV_WANDER = 100000
GM_QRF_TICK = 999999
GM_PANIC_SCATTER_T = 100000
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
removeAllWeapons player
player setCaptive true
player setPos [7500, 5700, 0]
asSaveZone = gmZoneIndex "Village"
triAssertEq [gmOccupierSide, "EAST"]
triAssertEq [gmResistanceSide, "GUER"]
triAssertEq [gmAssailantSide, "WEST"]
triAssertEq [gmAssailantCount "ALL", 0]
asSaveOldGroup = [asSaveZone] call GM_fnCivSpawnTown
asSaveResister = (units asSaveOldGroup) select 0
asSaveRogue = (units asSaveOldGroup) select 1
asSaveBystander = (units asSaveOldGroup) select 2
{_x disableAI "TARGET"; _x disableAI "AUTOTARGET"; _x disableAI "MOVE"; _x stop true; _x allowDammage false} forEach (units asSaveOldGroup)
asSaveResister setPos [7480, 5700, 0]
asSaveResister setDir 90
asSaveRogue setPos [7000, 5700, 0]
asSaveBystander setPos [7100, 5700, 0]
asSaveResType = typeOf asSaveResister
asSaveRogueType = typeOf asSaveRogue
triAssert [[asSaveResister, "RESISTER", player, "Village"] call GM_AS_fnBegin]
triAssert [[asSaveRogue, "ROGUE", objNull, "Village"] call GM_AS_fnBegin]
asSaveResGroup = group asSaveResister
asSaveRogueGroup = group asSaveRogue
asSaveResGroup setCombatMode "BLUE"
asSaveRogueGroup setCombatMode "BLUE"
triAssert [not (asSaveResGroup == asSaveRogueGroup)]
triAssert [not (asSaveResGroup == asSaveOldGroup)]
triAssert [not (asSaveRogueGroup == asSaveOldGroup)]
triAssertEq [count (units asSaveOldGroup), 1]
triAssertEq [count (weapons asSaveResister), 1]
triAssertEq [count (magazines asSaveResister), 3]
triAssertEq [count (weapons asSaveRogue), 1]
triAssertEq [count (magazines asSaveRogue), 3]
// A deliberately depleted inventory distinguishes restoration from re-arming.
asSaveResister removeMagazine ((magazines asSaveResister) select 0)
asSaveRogue removeMagazine ((magazines asSaveRogue) select 0)
triAssertEq [count (magazines asSaveResister), 2]
triAssertEq [count (magazines asSaveRogue), 2]
asSaveResWeapons = format ["%1", weapons asSaveResister]
asSaveResMags = format ["%1", magazines asSaveResister]
asSaveRogueWeapons = format ["%1", weapons asSaveRogue]
asSaveRogueMags = format ["%1", magazines asSaveRogue]
asSaveShots = 0
asSaveResister addEventHandler ["fired", {asSaveShots = asSaveShots + 1}]
// Leave a casualty in the native registry at the exact save boundary. Its
// dead brain and group links must be resolved before post-load retirement.
asSaveDeadOldGroup = createGroup GM_CIV_SIDE
(GM_CIV_CLASSES select 0) createUnit [[7300,5700,0],asSaveDeadOldGroup,"asSaveDead = this",0.5,"PRIVATE"]
asSaveDead disableAI "TARGET"
asSaveDead disableAI "AUTOTARGET"
asSaveDead disableAI "MOVE"
asSaveDead stop true
triAssert [gmAssailantRegister [asSaveDead, "RESISTER", player, "Village"]]
asSaveDeadGroup = group asSaveDead
asSaveDeadGroup setCombatMode "BLUE"
asSaveDeadType = typeOf asSaveDead
asSaveDeadClassAtDeath = ""
asSaveDead addEventHandler ["killed", {asSaveDeadClassAtDeath = gmAssailantClass (_this select 0)}]
asSavePanicUntil = GM_PANIC_UNTIL select (GM_PANIC_ZONES find "Village")
asSaveTicks = GM_AS_TICKS
triAssertEq [gmAssailantCount "RESISTER", 2]
triAssertEq [gmAssailantCount "ROGUE", 1]
// Keep death and serialization in one evaluator call: no simulation or
// one-second assailant prune can run between these operations.
triAssertEq [([] call {asSaveDead setDammage 1; asSaveDeadClassAtSave = gmAssailantClass asSaveDead; triSaveGame "assailants"}), "OK"]
triEndTest
