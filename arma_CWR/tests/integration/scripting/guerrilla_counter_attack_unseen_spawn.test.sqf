// ============================================================================
//  Occupier counter-attack, UNSEEN SPAWN (the user's rule of 2026-10-13: a
//  counter-attack always walks from a base where its spawn is not seen by
//  the player to the town it retakes; native CounterAttack service,
//  guerrilla-mode/OFFSCREEN-CONTRACT.md section 9).  Boots the SHIPPED
//  Guerrilla.Abel template on the full Remaster:
//    u1  the Outpost's garrison is wiped (it can spare nobody), Houdan's
//        garrison is killed and a fighter raises the town while the player
//        stands in Houdan looking toward the Outpost; the flip schedules the
//        first-town answer (its 0.4 roll tuned to 1);
//    u2  the nearest base that can spare men is the Le Port Harbour (2.7 km,
//        a stored reserve): the town is in the player's bubble, so the force
//        goes live at once, at its own base, because the player cannot see
//        it there: farther than the audible floor (trafficMinSpawnDist 300)
//        and outside the player's view cone (the Harbour is ~90 deg off the
//        Outpost bearing);
//    u3  it closes on Houdan over time, on the road.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
usT0 = time
triSimUntil { time > usT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

usRes = gmResistanceSide call GM_fnSideFromString
usV = gmZoneIndex "Village"
usO = gmZoneIndex "Outpost"
triAssertGe [usV, 0]
triAssertGe [usO, 0]
usPos = (gmZone usV) select 8
usOPos = (gmZone usO) select 8
triAssertEq [(format ["%1", gmCounterAttackTune ["delayMin", 5]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["delayMax", 10]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 1]]), "true"]
triAssertEq [(count gmCounterAttacks), 0]

// both garrisons are live: the player stands at the Camp, inside both bubbles
triSimUntil { gmGarrisonSpawned usV }
triSimUntil { (gmGarrisonLive usV) >= 4 }
triSimUntil { gmGarrisonSpawned usO }
triSimUntil { (gmGarrisonLive usO) >= 6 }

// ---- u1: the Outpost can spare nobody; raise Houdan with the player in it --
usUnits = []
"usUnits = usUnits + (units _x)" forEach (gmGarrisonGroups usO)
"usUnits = usUnits + (units _x)" forEach (gmGarrisonGroups usV)
{_x setDamage 1} forEach usUnits
triSimUntil { (gmGarrisonLive usV) == 0 }
triSimUntil { (gmGarrisonLive usO) == 0 }
triAssertEq [((gmZone usO) select 3), 0]
player setPos [(usPos select 0) + 3, (usPos select 1) + 3, 0]
// face the Outpost
usLook = ((usOPos select 0) - (usPos select 0)) atan2 ((usOPos select 1) - (usPos select 1))
player setDir usLook
usSq = [usRes, gmFactionSquad [gmResistanceSide, 1, 1], usPos] call GM_fnSpawnSquad
usSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units usSq)
doStop (units usSq)
triSimUntil { ((gmZone usV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units usSq)
usFlipT = time
triAssertEq [(count gmCounterAttacks), 1]

// ---- u2: LIVE at its own base, unseen ---------------------------------------
triSimUntil { player setDir usLook; (((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2) == "LIVE" }
usRow = gmCounterAttacks select 0
triAssertEq [(usRow select 1), "Le Port Harbour"]
triAssertEq [(usRow select 9), "base"]
usGrp = usRow select 7
triAssertGe [(count units usGrp), 1]
usSpawn = usRow select 8
triAssertEq [(count usSpawn), 3]
usBase = (gmZone (gmZoneIndex "Le Port Harbour")) select 8
// at the base: on its road, inside the 300 m road snap
triAssertGe [300, (usSpawn distance usBase)]
// the first position seen: farther than the audible floor ...
usFirst = getPos (leader usGrp)
triAssertGe [(usFirst distance usSpawn), 0]
triAssertGe [25, (usFirst distance usSpawn)]
usDx = (usSpawn select 0) - ((getPos player) select 0)
usDy = (usSpawn select 1) - ((getPos player) select 1)
usDist = sqrt (usDx * usDx + usDy * usDy)
triAssertGe [usDist, 300]
// ... and outside the view cone (+-60 deg of the player's heading)
usCos = (usDx * (sin (getDir player)) + usDy * (cos (getDir player))) / usDist
triAssertGe [0.5, usCos]
gmSimDiagLog

// ---- u3: it closes on the town, by road --------------------------------------
usD0 = (getPos (leader usGrp)) distance usPos
// it sets off (time-boxed so a failure reports the distances)
usT3 = time
triSimUntil { (((getPos (leader usGrp)) distance usSpawn) > 30) or (time > usT3 + 140) }
triAssertGe [((getPos (leader usGrp)) distance usSpawn), 30]
usT4 = time
triSimUntil { (((getPos (leader usGrp)) distance usPos) < (usD0 - 50)) or (time > usT4 + 140) }
triAssertGe [(usD0 - 50), ((getPos (leader usGrp)) distance usPos)]
// the column is on the road net (gmRoadNearest: a road within 20 m)
triSimUntil { (count (gmRoadNearest (getPos (leader usGrp)))) > 0 }
triAssertEq [(((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2), "LIVE"]

triEndTest
