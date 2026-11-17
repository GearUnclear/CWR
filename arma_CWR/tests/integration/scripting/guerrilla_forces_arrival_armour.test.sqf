// ============================================================================
//  Occupier forces, an OFF-SCREEN ARRIVAL-LIVE COLUMN keeps its armour
//  (OCCUPIER-FORCES.md 6.5 "At the town edge" and "Resolution: always a real
//  fight against present defenders", contract row 2c).  Video 4 of
//  2026-10-16: a truck plus T72 column and a BMP plus BMP column arrived
//  abstract at a defended town far from the player and were born as four
//  riflemen, their hulls held back, so the town repelled a force that
//  outgunned it on paper.  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1) on the full Remaster:
//    a1  a real camera at the default view distance in daylight; Houdan (the
//        "Village" CITY zone) loses its garrison and is handed to the
//        resistance; the player walks to the western hills, far from Houdan,
//        the Le Port Harbour and the road between them; three resistance
//        fighters stand in Houdan (shielded and holding fire for now) and
//        the zone census counts them;
//    a2  a funded column from the Harbour through the test lever
//        gmCounterAttackLaunchFunded: a Ural truck carries six infantry, a
//        BMP escorts it.  The abstract leg runs at the test speed; the
//        column arrives ABSTRACT (TRAVEL until then, never born on the way);
//    a3  the arrival against defenders is born LIVE at the town edge (the
//        last route point past the 250 m bound, spawn reason "route", far
//        from the player) WITH both hulls and their crews:
//        gmOccupierOps [8] reads 2, gmCounterAttacks [18] and [23] are live
//        hulls, [14] is 1;
//    a4  the truck stops and the infantry gets out; the escort drives up to
//        the town edge and supports: within 250 m of the edge (zoneArea 150,
//        so 400 m of the centre), crewed and alive;
//    a5  the fighters are released (vulnerable, free to fire): the fight
//        resolves live, the operation ends, and the town is no longer the
//        resistance's.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded; the legacy flip answer is off.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
faT0 = time
triSimUntil { time > faT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- a1: a real camera; Houdan handed over; the player far; three fighters -
faB = gmTrafficPercept (getPos player)
triAssertEq [(faB select 0), 1]
triAssertGe [(faB select 3), 750]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
faRes = gmResistanceSide call GM_fnSideFromString
faV = gmZoneIndex "Village"
faH = gmZoneIndex "Le Port Harbour"
triAssertGe [faV, 0]
triAssertGe [faH, 0]
faPos = (gmZone faV) select 8
faHPos = (gmZone faH) select 8
// the town garrison is live (the player starts at the Camp): kill it, then
// hand the town to the resistance
triSimUntil { gmGarrisonSpawned faV }
triSimUntil { (gmGarrisonLive faV) >= 4 }
faUnits = []
"faUnits = faUnits + (units _x)" forEach (gmGarrisonGroups faV)
{_x setDamage 1} forEach faUnits
triSimUntil { (gmGarrisonLive faV) == 0 }
gmZoneSet [faV, "garrison", 0]
gmZoneSet [faV, "owner", gmResistanceSide]
gmZoneSet [faV, "support", 70]
triSimUntil { ((gmZone faV) select 2) == gmResistanceSide }
// the western hills: far from Houdan, the Harbour and the road between them
faFar = [4500, 5200, 0]
if ((gmLandmass faFar) < 0) then {faFar = [7922, 8500, 0]}
triAssertGe [(faFar distance faPos), 1500]
triAssertGe [(faFar distance faHPos), 1500]
player setPos faFar
triSimUntil { not (gmGarrisonSpawned faV) }
triSimUntil { not (gmGarrisonSpawned faH) }
triAssertGe [((gmZone faH) select 3), 6]
gmZoneSet [faV, "garrison", 0]
faSq = [faRes, gmFactionSquad [gmResistanceSide, 1, 3], faPos] call GM_fnSpawnSquad
faSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units faSq)
doStop (units faSq)
triAssertEq [(count units faSq), 3]
triSimUntil { ((gmZoneSim faV) select 1) >= 3 }
triAssertEq [((gmZone faV) select 2), gmResistanceSide]
triAssertEq [((gmTrafficPercept (getPos player)) select 0), 1]

// ---- a2: a funded column, abstract on the road --------------------------------
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 40]]), "true"]
faOp = gmCounterAttackLaunchFunded ["Village", "Le Port Harbour", "truck", 6, true, "Ural", "RETAKE", "BMP"]
triAssertGe [faOp, 1]
triAssertEq [(count gmOccupierOps), 1]
faNone = [0, "", "", "", "", "", "NONE", 0, 0, 0, "", "", ""]
faRow = gmOccupierOps select 0
triAssertEq [(faRow select 0), faOp]
triAssertEq [(faRow select 3), "TRUCK"]
triAssertEq [(faRow select 6), "TRAVEL"]
triAssertEq [(faRow select 11), "Ural"]
triAssertEq [(faRow select 12), "BMP"]
// abstract on the road until it arrives (a3 checks it was born at the edge)
faT1 = time
triSimUntil { faRow = (gmOccupierOps + [faNone]) select 0; ((faRow select 6) != "TRAVEL") or (time > faT1 + 300) }
triAssertEq [(faRow select 6), "LIVE"]
triAssertEq [(faRow select 0), faOp]

// ---- a3: born live at the town edge WITH its carrier and escort ---------------
triAssertEq [(faRow select 8), 2]
faCa = gmCounterAttacks select 0
triAssertEq [(faCa select 10), faOp]
triAssertEq [(faCa select 2), "LIVE"]
triAssertEq [(faCa select 9), "route"]
triAssertEq [(faCa select 14), 1]
triAssertEq [(faCa select 22), "BMP"]
faHull = faCa select 18
faEsc = faCa select 23
triAssertEq [(format ["%1", isNull faHull]), "false"]
triAssertEq [(format ["%1", isNull faEsc]), "false"]
triAssertGe [(count crew faHull), 1]
triAssertGe [(count crew faEsc), 1]
// the arrival birth point: the last route point past the 250 m bound (an
// on-the-way birth would stand back on the road), far from the player
faSpawn = faCa select 8
triAssertGe [(faSpawn distance faPos), 245]
triAssertGe [500, (faSpawn distance faPos)]
triAssertGe [(faSpawn distance player), 1000]
triAssertGe [(faEsc distance player), 1000]
triAssertGe [60, (faEsc distance faHull)]
gmSimDiagLog

// ---- a4: the infantry gets out; the escort supports at the town edge ----------
faNone2 = [[], [], "NONE"]
faT2 = time
triSimUntil { ((((gmCounterAttacks + [faNone2]) select 0) select 2) != "LIVE") or ((((gmCounterAttacks + [faNone2]) select 0) select 19) == 0) or (time > faT2 + 240) }
faCa = gmCounterAttacks select 0
triAssertEq [(faCa select 2), "LIVE"]
triAssertEq [(faCa select 19), 0]
triAssertGe [(faCa select 21), 1]
faT3 = time
triSimUntil { ((faEsc distance faPos) < 400) or (time > faT3 + 240) }
triAssertGe [400, (faEsc distance faPos)]
triAssertEq [(format ["%1", alive faEsc]), "true"]
triAssertGe [(count crew faEsc), 1]
triAssertEq [(format ["%1", alive faHull]), "true"]
triAssertEq [(((gmOccupierOps + [faNone]) select 0) select 8), 2]
gmSimDiagLog

// ---- a5: the fighters fight; the fight resolves live --------------------------
{_x allowDammage true} forEach (units faSq)
faSq setCombatMode "RED"
faT4 = time
faLeft = 1
triSimUntil { faLeft = 0; {if ((_x select 10) == faOp) then {faLeft = faLeft + 1}} forEach gmCounterAttacks; (faLeft == 0) or (time > faT4 + 420) }
triAssertEq [faLeft, 0]
triAssertEq [(format ["%1", ((gmZone faV) select 2) != gmResistanceSide]), "true"]
gmSimDiagLog

triEndTest
