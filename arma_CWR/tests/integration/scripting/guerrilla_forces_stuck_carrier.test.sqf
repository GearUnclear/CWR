// ============================================================================
//  Occupier forces, a CARRIER BORN NEAR A DEFENDED TOWN dismounts and its
//  infantry reaches the town (OCCUPIER-FORCES.md 6.5 "At the town edge",
//  "Arrival births").  Video 6 of 2026-10-17: op 13, a Ural with nine men
//  and a T72 from the Outpost, arrived abstract at a defended Sainte Marie
//  (the road winds 7102 m for 2913 m straight, so the leg is cross-country),
//  was born at the last route point past the 250 m bound, 297 m from the
//  centre, and sat there with every man seated for 1617 s until the live
//  timeout withdrew it.  Boots the SHIPPED Guerrilla.Abel template
//  (occupierForces = 1) on the full Remaster:
//    a1  a real camera at the default view distance in daylight; Sainte
//        Marie is the resistance's with six fighters standing in it
//        (shielded, holding fire); the player is far from the town, the
//        Outpost and the line between them;
//    a2  a funded column from the Outpost through the test lever
//        gmCounterAttackLaunchFunded: a Ural carries five infantry (the Outpost's spare), a T72
//        escorts it; abstract on the leg, it arrives ABSTRACT and is born
//        LIVE at the arrival point, within 330 m of the centre, with both
//        hulls (the video's birth);
//    a3  within 150 s of the birth the infantry is out of the carrier
//        (gmCounterAttacks [27] dismounted, [19] no one seated), whether the
//        carrier drove to the 250 m mark or stalled where it was born;
//    a4  within 300 s of the dismount the infantry stands in the town
//        (a living man of the squad within zoneArea of the centre), and the
//        operation is still LIVE (no live-timeout withdrawal).
//  The companion roster and the QRF director are parked; the player and the
//  hulls are damage-shielded; the legacy flip answer is off.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
scT0 = time
triSimUntil { time > scT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- a1: a real camera; Sainte Marie handed over; the player far ----------
scB = gmTrafficPercept (getPos player)
triAssertEq [(scB select 0), 1]
triAssertGe [(scB select 3), 750]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
scRes = gmResistanceSide call GM_fnSideFromString
scV = gmZoneIndex "Sainte Marie"
scO = gmZoneIndex "Outpost"
triAssertGe [scV, 0]
triAssertGe [scO, 0]
scPos = (gmZone scV) select 8
scOPos = (gmZone scO) select 8
// the north, near La Trinite: far from Sainte Marie, the Outpost and the line
scFar = [7922, 8500, 0]
triAssertGe [(scFar distance scPos), 3000]
triAssertGe [(scFar distance scOPos), 1500]
player setPos scFar
triSimUntil { not (gmGarrisonSpawned scV) }
triSimUntil { not (gmGarrisonSpawned scO) }
triAssertGe [((gmZone scO) select 3), 6]
gmZoneSet [scV, "garrison", 0]
gmZoneSet [scV, "owner", gmResistanceSide]
gmZoneSet [scV, "support", 70]
triSimUntil { ((gmZone scV) select 2) == gmResistanceSide }
gmZoneSet [scV, "garrison", 0]
scSq = [scRes, gmFactionSquad [gmResistanceSide, 1, 6], scPos] call GM_fnSpawnSquad
scSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units scSq)
doStop (units scSq)
triAssertEq [(count units scSq), 6]
triSimUntil { ((gmZoneSim scV) select 1) >= 6 }
triAssertEq [((gmZone scV) select 2), gmResistanceSide]
triAssertEq [((gmTrafficPercept (getPos player)) select 0), 1]

// ---- a2: a funded column, abstract, born live at the arrival point ---------
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 40]]), "true"]
scOp = gmCounterAttackLaunchFunded ["Sainte Marie", "Outpost", "truck", 5, true, "Ural", "RETAKE", "T72"]
triAssertGe [scOp, 1]
scNone = [[], [], "NONE", 0, 0, 0, 0, grpNull, [], "", 0, 0, 0, 0, 0, 0, "", 0, objNull, 0, objNull, 0, "", objNull, 0, [], 0, false, false]
scRow = scNone
scT1 = time
triSimUntil { scRow = scNone; {if ((_x select 10) == scOp) then {scRow = _x}} forEach gmCounterAttacks; ((scRow select 2) == "LIVE") or (time > scT1 + 300) }
triAssertEq [(scRow select 2), "LIVE"]
triAssertEq [(scRow select 9), "route"]
triAssertEq [(scRow select 14), 1]
triAssertEq [(scRow select 22), "T72"]
scHull = scRow select 18
scEsc = scRow select 23
triAssertEq [(format ["%1", isNull scHull]), "false"]
triAssertEq [(format ["%1", isNull scEsc]), "false"]
scHull allowDammage false
scEsc allowDammage false
{_x allowDammage false} forEach (crew scHull)
{_x allowDammage false} forEach (crew scEsc)
scSpawn = scRow select 8
logInfo format ["SCBORN %1", [time, scSpawn, scSpawn distance scPos, getPos scHull, getPos scEsc, scRow select 19]]
triAssertGe [330, (scSpawn distance scPos)]
triAssertGe [(scSpawn distance scPos), 245]
triAssertGe [(scSpawn distance player), 1000]
gmSimDiagLog

// ---- a3: the infantry gets out within 150 s of the birth -------------------
scT2 = time
scDiag = time
triSimUntil { scRow = scNone; {if ((_x select 10) == scOp) then {scRow = _x}} forEach gmCounterAttacks; if (time > scDiag + 10) then {scDiag = time; logInfo format ["SCDIAG %1", [time, getPos scHull, speed scHull, canMove scHull, fuel scHull, getDammage scHull, alive (driver scHull), count (crew scHull), getPos scEsc, scRow select 19, scRow select 27]]}; ((scRow select 2) != "LIVE") or ((scRow select 27) and ((scRow select 19) == 0)) or (time > scT2 + 150) }
triAssertEq [(scRow select 2), "LIVE"]
triAssertEq [(format ["%1", scRow select 27]), "true"]
triAssertEq [(scRow select 19), 0]
scG = scRow select 7
triAssertEq [(format ["%1", isNull scG]), "false"]
logInfo format ["SCOUT %1", [time - scT2, getPos scHull, (getPos scHull) distance scPos]]
gmSimDiagLog

// ---- a4: the infantry reaches the town within 300 s of the dismount --------
// zoneArea: the Abel template keeps the stock 150 m
scArea = 150
scT3 = time
scIn = 0
triSimUntil { scIn = 0; {if ((alive _x) and ((_x distance scPos) <= scArea)) then {scIn = scIn + 1}} forEach (units scG); scRow = scNone; {if ((_x select 10) == scOp) then {scRow = _x}} forEach gmCounterAttacks; (scIn > 0) or ((scRow select 2) != "LIVE") or (time > scT3 + 300) }
logInfo format ["SCIN %1", [time - scT3, scIn, scArea]]
triAssertGe [scIn, 1]
triAssertEq [(scRow select 2), "LIVE"]
gmSimDiagLog

triEndTest
