// ============================================================================
//  Occupier forces, the LONE SURVIVOR of an assault BREAKS OFF at once
//  (decision 4, the feel assessment's item 7, NEXT question 2).  Video 5 of
//  2026-10-16: ops 12 and 15 each left one man 47 m from Arudy for 18
//  minutes, until counterAttackLiveTimeout.  Boots the SHIPPED Guerrilla.Abel
//  template (occupierForces = 1) on the full Remaster:
//    l1  a real camera; Houdan (the "Village" CITY zone) handed to the
//        resistance; the player on the western hills; four resistance
//        fighters in Houdan, shielded and holding fire;
//    l2  a funded foot squad of four from the Harbour through the test
//        lever; it arrives abstract and is born LIVE at the town edge;
//    l3  at the edge (the live clock running) the player comes to 300 m
//        beyond Houdan (captive); three of the four die;
//    l4  the one man left breaks off within seconds, long before the live
//        timeout: a repel, reason "broken off", the line "Broken off at
//        Village: 3 lost, 1 going home"; no hull to wait for; he walks home
//        as a withdrawing Leftover, his distance from Houdan growing;
//    l5  the player leaves: he leaves the world unseen.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
blT0 = time
triSimUntil { time > blT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- l1: a real camera; Houdan handed over; the player far; four fighters --
blB = gmTrafficPercept (getPos player)
triAssertEq [(blB select 0), 1]
triAssertGe [(blB select 3), 750]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
blRes = gmResistanceSide call GM_fnSideFromString
blV = gmZoneIndex "Village"
blH = gmZoneIndex "Le Port Harbour"
triAssertGe [blV, 0]
triAssertGe [blH, 0]
blPos = (gmZone blV) select 8
blHPos = (gmZone blH) select 8
triSimUntil { gmGarrisonSpawned blV }
triSimUntil { (gmGarrisonLive blV) >= 4 }
blUnits = []
"blUnits = blUnits + (units _x)" forEach (gmGarrisonGroups blV)
{_x setDamage 1} forEach blUnits
triSimUntil { (gmGarrisonLive blV) == 0 }
gmZoneSet [blV, "garrison", 0]
gmZoneSet [blV, "owner", gmResistanceSide]
gmZoneSet [blV, "support", 70]
triSimUntil { ((gmZone blV) select 2) == gmResistanceSide }
blFar = [4500, 5200, 0]
if ((gmLandmass blFar) < 0) then {blFar = [7922, 8500, 0]}
triAssertGe [(blFar distance blPos), 1500]
triAssertGe [(blFar distance blHPos), 1500]
player setPos blFar
triSimUntil { not (gmGarrisonSpawned blV) }
triSimUntil { not (gmGarrisonSpawned blH) }
triAssertGe [((gmZone blH) select 3), 4]
gmZoneSet [blV, "garrison", 0]
blSq = [blRes, gmFactionSquad [gmResistanceSide, 1, 4], blPos] call GM_fnSpawnSquad
blSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units blSq)
doStop (units blSq)
triAssertEq [(count units blSq), 4]
triSimUntil { ((gmZoneSim blV) select 1) >= 4 }
triAssertEq [((gmZone blV) select 2), gmResistanceSide]

// ---- l2: a funded foot squad, born live at the town edge ----------------------
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 40]]), "true"]
blOp = gmCounterAttackLaunchFunded ["Village", "Le Port Harbour", "foot", 4, true]
triAssertGe [blOp, 1]
blNone = [[], [], "NONE", 0, 0, 0, 0, grpNull, [], "", 0, 0, 0, 0, 0, 0, "", 0, objNull, 0, objNull, 0, "", objNull, 0, [], 0, false, false]
blRow = blNone
blT1 = time
triSimUntil { blRow = blNone; {if ((_x select 10) == blOp) then {blRow = _x}} forEach gmCounterAttacks; ((blRow select 2) == "LIVE") or (time > blT1 + 300) }
triAssertEq [(blRow select 2), "LIVE"]
triAssertEq [(blRow select 14), 0]
// at the edge: the live clock counts from there (negative on the road)
blT2 = time
triSimUntil { blRow = blNone; {if ((_x select 10) == blOp) then {blRow = _x}} forEach gmCounterAttacks; ((blRow select 2) != "LIVE") or ((blRow select 3) >= 0) or (time > blT2 + 240) }
triAssertEq [(blRow select 2), "LIVE"]
triAssertGe [(blRow select 3), 0]
blG = blRow select 7
triAssertEq [(format ["%1", isNull blG]), "false"]
blBorn = 0
{if (alive _x) then {blBorn = blBorn + 1}} forEach (units blG)
triAssertEq [blBorn, 4]

// ---- l3: the player 300 m beyond Houdan; three of the four die -----------------
blDx = (blPos select 0) - (blHPos select 0)
blDz = (blPos select 1) - (blHPos select 1)
blD = sqrt ((blDx * blDx) + (blDz * blDz))
blNear = [(blPos select 0) + (blDx / blD) * 300, (blPos select 1) + (blDz / blD) * 300, 0]
if ((gmLandmass blNear) < 0) then {blNear = [(blPos select 0), (blPos select 1), 0]}
player setCaptive true
player setPos blNear
blT3 = time
triSimUntil { time > blT3 + 3 }
blK = 0
{if ((alive _x) and (blK < 3)) then {_x setDamage 1; blK = blK + 1}} forEach (units blG)
triAssertEq [blK, 3]
blSurv = objNull
{if (alive _x) then {blSurv = _x}} forEach (units blG)
triAssertEq [(format ["%1", alive blSurv]), "true"]
blClock = 0
{if ((_x select 10) == blOp) then {blClock = _x select 3}} forEach gmCounterAttacks

// ---- l4: he breaks off at once, long before the live timeout ---------------------
blT4 = time
blLeft = 1
triSimUntil { blLeft = 0; {if ((_x select 10) == blOp) then {blLeft = blLeft + 1}} forEach gmCounterAttacks; (blLeft == 0) or (time > blT4 + 30) }
triAssertEq [blLeft, 0]
triAssertGe [600, blClock + (time - blT4)]
blEnd = [0, "", "", "", 0, 0, 0, "", ""]
{if ((_x select 0) == blOp) then {blEnd = _x}} forEach gmCounterAttackEnds
triAssertEq [(blEnd select 2), "REPELLED"]
triAssertEq [(blEnd select 3), "broken off"]
triAssertEq [(blEnd select 4), 1]
triAssertEq [(blEnd select 5), 3]
triAssertEq [(blEnd select 6), 4]
triAssertEq [(blEnd select 8), "Broken off at Village: 3 lost, 1 going home"]
// no hull to wait for; he walks home as a withdrawing Leftover
blHH = 0
{if ((_x select 0) == blOp) then {blHH = blHH + 1}} forEach gmCounterAttackHomeHulls
triAssertEq [blHH, 0]
blLo = [grpNull, 0, false, objNull]
{if ((_x select 0) == blG) then {blLo = _x}} forEach gmCounterAttackLeftovers
triAssertEq [(format ["%1", blLo select 2]), "true"]
triAssertEq [(blLo select 1), 1]
blD0 = blSurv distance blPos
blT5 = time
triSimUntil { ((blSurv distance blPos) > (blD0 + 25)) or (not (alive blSurv)) or (time > blT5 + 180) }
triAssertEq [(format ["%1", alive blSurv]), "true"]
triAssertGe [(blSurv distance blPos), blD0 + 25]
gmSimDiagLog

// ---- l5: the player leaves; he leaves the world unseen -----------------------------
player setPos blFar
blT6 = time
blLoLeft = 1
triSimUntil { blLoLeft = 0; {if ((_x select 0) == blG) then {blLoLeft = blLoLeft + 1}} forEach gmCounterAttackLeftovers; (blLoLeft == 0) or (time > blT6 + 180) }
triAssertEq [blLoLeft, 0]
gmSimDiagLog

triEndTest
