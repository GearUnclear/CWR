// ============================================================================
//  Occupier forces, a FAILED COLUMN ASSAULT BREAKS OFF and its hulls leave
//  AFTER the survivors (decision 4, the feel assessment's item 7;
//  OCCUPIER-FORCES.md 6.5 "At the town edge", contract row 2c and 9.6).
//  Video 5 of 2026-10-16: hulls drove home while their riflemen died, and a
//  lone survivor stood 47 m from Arudy until the 1200 s live timeout.  Boots
//  the SHIPPED Guerrilla.Abel template (occupierForces = 1) on the full
//  Remaster:
//    b1  a real camera at the default view distance in daylight; Houdan (the
//        "Village" CITY zone) loses its garrison and is handed to the
//        resistance; the player walks to the western hills; four resistance
//        fighters stand in Houdan, shielded and holding fire, and the zone
//        census counts them;
//    b2  a funded column from the Harbour through the test lever: a Ural
//        carries six infantry, a BMP escorts it; it arrives abstract and is
//        born LIVE at the town edge with both hulls (the arrival rule);
//    b3  the truck stops and the infantry gets out; the hulls are shielded;
//        the player stands 300 m beyond Houdan (captive), so nothing the
//        column leaves can vanish under him;
//    b4  five of the six riflemen die: living infantry below a third of the
//        six born while the town is still the resistance's.  The operation
//        ends at once as a repel with the reason "broken off" and the line
//        "Broken off at Village: 5 lost, 1 going home"; the survivor walks
//        home as a withdrawing Leftover; both hulls HOLD for him;
//    b5  the hulls leave only once he has cleared 250 m of Houdan, or after
//        the 120 s bound: never before; while they hold they do not drive
//        off; then they drive home;
//    b6  the player leaves: the survivor leaves the world unseen.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded; the legacy flip answer is off.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
boT0 = time
triSimUntil { time > boT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- b1: a real camera; Houdan handed over; the player far; four fighters --
boB = gmTrafficPercept (getPos player)
triAssertEq [(boB select 0), 1]
triAssertGe [(boB select 3), 750]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
boRes = gmResistanceSide call GM_fnSideFromString
boV = gmZoneIndex "Village"
boH = gmZoneIndex "Le Port Harbour"
triAssertGe [boV, 0]
triAssertGe [boH, 0]
boPos = (gmZone boV) select 8
boHPos = (gmZone boH) select 8
triSimUntil { gmGarrisonSpawned boV }
triSimUntil { (gmGarrisonLive boV) >= 4 }
boUnits = []
"boUnits = boUnits + (units _x)" forEach (gmGarrisonGroups boV)
{_x setDamage 1} forEach boUnits
triSimUntil { (gmGarrisonLive boV) == 0 }
gmZoneSet [boV, "garrison", 0]
gmZoneSet [boV, "owner", gmResistanceSide]
gmZoneSet [boV, "support", 70]
triSimUntil { ((gmZone boV) select 2) == gmResistanceSide }
boFar = [4500, 5200, 0]
if ((gmLandmass boFar) < 0) then {boFar = [7922, 8500, 0]}
triAssertGe [(boFar distance boPos), 1500]
triAssertGe [(boFar distance boHPos), 1500]
player setPos boFar
triSimUntil { not (gmGarrisonSpawned boV) }
triSimUntil { not (gmGarrisonSpawned boH) }
triAssertGe [((gmZone boH) select 3), 6]
gmZoneSet [boV, "garrison", 0]
boSq = [boRes, gmFactionSquad [gmResistanceSide, 1, 4], boPos] call GM_fnSpawnSquad
boSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units boSq)
doStop (units boSq)
triAssertEq [(count units boSq), 4]
triSimUntil { ((gmZoneSim boV) select 1) >= 4 }
triAssertEq [((gmZone boV) select 2), gmResistanceSide]

// ---- b2: a funded column, born live at the town edge -------------------------
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 40]]), "true"]
boOp = gmCounterAttackLaunchFunded ["Village", "Le Port Harbour", "truck", 6, true, "Ural", "RETAKE", "BMP"]
triAssertGe [boOp, 1]
boNone = [[], [], "NONE", 0, 0, 0, 0, grpNull, [], "", 0, 0, 0, 0, 0, 0, "", 0, objNull, 0, objNull, 0, "", objNull, 0, [], 0, false, false]
boRow = boNone
boT1 = time
triSimUntil { boRow = boNone; {if ((_x select 10) == boOp) then {boRow = _x}} forEach gmCounterAttacks; ((boRow select 2) == "LIVE") or (time > boT1 + 300) }
triAssertEq [(boRow select 2), "LIVE"]
triAssertEq [(boRow select 14), 1]
triAssertEq [(boRow select 22), "BMP"]
boHull = boRow select 18
boEsc = boRow select 23
triAssertEq [(format ["%1", isNull boHull]), "false"]
triAssertEq [(format ["%1", isNull boEsc]), "false"]
// the hulls and their crews are shielded: this test is about who leaves
// first, not about a hull lost to the defenders.  Only the crews: the
// infantry still rides in the Ural here and "crew" lists the riders too, so
// a blanket shield made the b4 setDamage kills no-ops (alive 6 of 6, never
// broken off)
boHull allowDammage false
boEsc allowDammage false
{if (not ((group _x) == (boRow select 7))) then {_x allowDammage false}} forEach (crew boHull)
{_x allowDammage false} forEach (crew boEsc)

// ---- b3: the infantry is out; the player 300 m beyond Houdan ----------------
boT2 = time
triSimUntil { boRow = boNone; {if ((_x select 10) == boOp) then {boRow = _x}} forEach gmCounterAttacks; ((boRow select 2) != "LIVE") or ((boRow select 27) and ((boRow select 19) == 0)) or (time > boT2 + 240) }
triAssertEq [(boRow select 2), "LIVE"]
triAssertEq [(format ["%1", boRow select 27]), "true"]
triAssertEq [(boRow select 19), 0]
boG = boRow select 7
triAssertEq [(format ["%1", isNull boG]), "false"]
boBorn = 0
{if (alive _x) then {boBorn = boBorn + 1}} forEach (units boG)
triAssertGe [boBorn, 4]
boDx = (boPos select 0) - (boHPos select 0)
boDz = (boPos select 1) - (boHPos select 1)
boD = sqrt ((boDx * boDx) + (boDz * boDz))
boNear = [(boPos select 0) + (boDx / boD) * 300, (boPos select 1) + (boDz / boD) * 300, 0]
if ((gmLandmass boNear) < 0) then {boNear = [(boPos select 0), (boPos select 1), 0]}
player setCaptive true
player setPos boNear
boT3 = time
triSimUntil { time > boT3 + 3 }
gmSimDiagLog

// ---- b4: all but one rifleman die: it breaks off ------------------------------
boKill = boBorn - 1
boK = 0
{if ((alive _x) and (boK < boKill)) then {_x setDamage 1; boK = boK + 1}} forEach (units boG)
triAssertEq [boK, boKill]
boSurv = objNull
{if (alive _x) then {boSurv = _x}} forEach (units boG)
triAssertEq [(format ["%1", alive boSurv]), "true"]
boT4 = time
boLeft = 1
triSimUntil { boLeft = 0; {if ((_x select 10) == boOp) then {boLeft = boLeft + 1}} forEach gmCounterAttacks; (boLeft == 0) or (time > boT4 + 30) }
triAssertEq [boLeft, 0]
// the operation ended as a repel, reason "broken off", with its line
boEnd = [0, "", "", "", 0, 0, 0, "", ""]
{if ((_x select 0) == boOp) then {boEnd = _x}} forEach gmCounterAttackEnds
triAssertEq [(boEnd select 2), "REPELLED"]
triAssertEq [(boEnd select 3), "broken off"]
triAssertEq [(boEnd select 4), 1]
triAssertGe [(boEnd select 6), boBorn]
triAssertEq [(boEnd select 8), (format ["Broken off at Village: %1 lost, 1 going home", (boEnd select 6) - 1])]
// the survivor walks home as a withdrawing Leftover
boLo = [grpNull, 0, false, objNull]
{if ((_x select 0) == boG) then {boLo = _x}} forEach gmCounterAttackLeftovers
triAssertEq [(format ["%1", boLo select 2]), "true"]
triAssertEq [(boLo select 1), 1]
// both hulls hold for him
boHH = []
{if ((_x select 0) == boOp) then {boHH = boHH + [_x]}} forEach gmCounterAttackHomeHulls
triAssertEq [(count boHH), 2]
{triAssertEq [(format ["%1", _x select 3]), "true"]} forEach boHH
boHullD0 = boHull distance boPos
boEscD0 = boEsc distance boPos
gmSimDiagLog

// ---- b5: the hulls leave only after him (cleared 250 m, or 120 s) -------------
boWaiting = 2
boSurvD = boSurv distance boPos
boHeldMax = 0
boT5 = time
triSimUntil { boWaiting = 0; {if (((_x select 0) == boOp) and (_x select 3)) then {boWaiting = boWaiting + 1}} forEach gmCounterAttackHomeHulls; if (alive boSurv) then {boSurvD = boSurv distance boPos}; if (boWaiting > 0) then {if (((boHull distance boPos) - boHullD0) > boHeldMax) then {boHeldMax = (boHull distance boPos) - boHullD0}; if (((boEsc distance boPos) - boEscD0) > boHeldMax) then {boHeldMax = (boEsc distance boPos) - boEscD0}}; (boWaiting == 0) or (time > boT5 + 200) }
triAssertEq [boWaiting, 0]
// while they held, neither drove off toward home
triAssertGe [60, boHeldMax]
boHH = []
{if ((_x select 0) == boOp) then {boHH = boHH + [_x]}} forEach gmCounterAttackHomeHulls
triAssertEq [(count boHH), 2]
boWhy = (boHH select 0) select 5
boWaited = (boHH select 0) select 4
boOk = false
if (boWhy == "cleared") then {boOk = boSurvD >= 240}
if (boWhy == "wait over") then {boOk = boWaited >= 119}
if (boWhy == "no infantry") then {boOk = not (alive boSurv)}
triAssertEq [(format ["%1 %2 %3", boWhy, boOk, boSurvD]), (format ["%1 true %2", boWhy, boSurvD])]
triAssertEq [((boHH select 1) select 5), boWhy]
// then they drive home (away from Houdan), or have already gone unseen
boT6 = time
boGone = false
triSimUntil { boGone = ((boHull distance boPos) > (boHullD0 + 40)) or (isNull boHull); boGone or (time > boT6 + 150) }
triAssertEq [(format ["%1", boGone]), "true"]
gmSimDiagLog

// ---- b6: the player leaves; the survivor leaves the world unseen --------------
player setPos boFar
boT7 = time
boLoLeft = 1
triSimUntil { boLoLeft = 0; {if ((_x select 0) == boG) then {boLoLeft = boLoLeft + 1}} forEach gmCounterAttackLeftovers; (boLoLeft == 0) or (time > boT7 + 180) }
triAssertEq [boLoLeft, 0]
gmSimDiagLog

triEndTest
