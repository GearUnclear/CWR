// ============================================================================
//  Occupier forces director across a save (OCCUPIER-FORCES.md phase 3,
//  section 11), phase 1 of 2: Guerrilla.Abel (occupierForces = 1), two towns
//  raised with the player 2.5 km off, the director wakes and launches, one
//  town is forced to rest (a repel's cooldown), then the game is saved.  The
//  expected values ride the save as script globals (fs*), so phase 2
//  compares against what this phase saw.  The director tick is left at its
//  60 s default after the launches, so nothing it owns moves between the
//  snapshot and the save.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
fsT0 = time
triSimUntil { time > fsT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]

fsRes = gmResistanceSide call GM_fnSideFromString
fsA = gmZoneIndex "Village"
triAssertGe [fsA, 0]
fsAPos = (gmZone fsA) select 8
fsB = -1
fsBest = 1000000
fsI = 0
while {fsI < gmZoneCount} do {fsZ = gmZone fsI; if (((fsZ select 1) == "CITY") and ((fsZ select 2) == "NEUTRAL") and (fsI != fsA) and (not ((gmZoneSim fsI) select 10))) then {fsD = (fsZ select 8) distance fsAPos; if (fsD < fsBest) then {fsBest = fsD; fsB = fsI}}; fsI = fsI + 1}
triAssertGe [fsB, 0]
fsBPos = (gmZone fsB) select 8
fsBName = (gmZone fsB) select 0

player setPos [7922, 8500, 0]
triSimUntil { not ((gmZoneSim fsA) select 0) }
triSimUntil { not ((gmZoneSim fsB) select 0) }
triSimUntil { not (gmGarrisonSpawned fsA) }
triSimUntil { not (gmGarrisonSpawned fsB) }

// ---- town 1 (DORMANT, legacy), then town 2 (AWAKE) --------------------------
gmZoneSet [fsA, "garrison", 0]
gmZoneSet [fsA, "support", 55]
fsSqA = [fsRes, gmFactionSquad [gmResistanceSide, 1, 1], fsAPos] call GM_fnSpawnSquad
fsSqA setCombatMode "BLUE"
{_x allowDammage false} forEach (units fsSqA)
doStop (units fsSqA)
triSimUntil { ((gmZone fsA) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units fsSqA)
gmZoneSet [fsA, "heatRaise", 20]
gmZoneSet [fsB, "garrison", 0]
gmZoneSet [fsB, "support", 55]
fsSqB = [fsRes, gmFactionSquad [gmResistanceSide, 1, 1], fsBPos] call GM_fnSpawnSquad
fsSqB setCombatMode "BLUE"
{_x allowDammage false} forEach (units fsSqB)
doStop (units fsSqB)
triSimUntil { ((gmZone fsB) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units fsSqB)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
fsOps = []
triSimUntil { fsOps = []; {if ((_x select 2) == "DIRECTOR") then {fsOps = fsOps + [_x]}} forEach gmOccupierOps; (count fsOps) >= 2 }

// ---- freeze the director, add a rest, snapshot, save ------------------------
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 600]]), "true"]
triAssertEq [(format ["%1", gmForcesForce ["cooldown", fsB, 1500]]), "true"]
fsRow = gmOccupierForces
triAssertEq [(count fsRow), 15]
fsPool = fsRow select 0
fsBand = fsRow select 2
fsTokens = fsRow select 3
fsEver = fsRow select 10
fsCool = fsRow select 12
fsCounter = fsRow select 13
triAssertEq [(fsRow select 4), "AWAKE"]
triAssertEq [fsTokens, 0]
triAssertEq [(count fsEver), 2]
triAssertEq [(format ["%1", "Village" in fsEver]), "true"]
triAssertEq [(format ["%1", fsBName in fsEver]), "true"]
triAssertEq [(count fsCool), 1]
triAssertEq [((fsCool select 0) select 0), fsBName]
triAssertGe [fsCounter, 2]
fsIds = []
{if ((_x select 2) == "DIRECTOR") then {fsIds = fsIds + [_x select 0]}} forEach gmOccupierOps
triAssertEq [(count fsIds), 2]
fsIntel = count gmOccupierIntel
fsSaveT = time
triAssertEq [(triSaveGame "forcesdir"), "OK"]

triEndTest
