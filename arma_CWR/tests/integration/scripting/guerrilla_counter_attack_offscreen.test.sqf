// ============================================================================
//  Occupier counter-attack, OFF-SCREEN path (issue #79 B2 follow-up; native
//  CounterAttack service, guerrilla-mode/OFFSCREEN-CONTRACT.md decision C).
//  Boots the SHIPPED Guerrilla.Abel template on the full Remaster and works
//  Houdan (the "Village" CITY zone) with the player 2.5 km away:
//    o1  Houdan's garrison is killed, one fighter is left in the town and the
//        player leaves; the Outpost caches back to its stored reserve of 6
//        and the town rises on the remote tick (D1) with nobody watching;
//    o2  the fighter walks out; the flip's counter-attack launches inside
//        the delay window (tuned to 5..10 s) as an ABSTRACT force: 5 of the
//        Outpost's 6 leave its stored reserve (1 stays home) and TRAVEL, no
//        entity is created;
//    o3  it arrives in the empty town (counterAttack event) and HOLDS it as
//        abstract occupier presence; the remote retake drain takes support
//        below townRevertSupport (30) and Houdan reverts to NEUTRAL with its
//        town garrison refilled as a stored reserve; the record is finished
//        and nothing returns to the Outpost.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
coT0 = time
triSimUntil { time > coT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

coRes = gmResistanceSide call GM_fnSideFromString
coV = gmZoneIndex "Village"
coO = gmZoneIndex "Outpost"
triAssertGe [coV, 0]
triAssertGe [coO, 0]
coPos = (gmZone coV) select 8
coFar = [7922, 8500, 0]
triAssertEq [(format ["%1", gmCounterAttackTune ["delayMin", 5]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["delayMax", 10]]), "true"]

// ---- o1: kill the town garrison, leave a fighter, walk away -----------------
triSimUntil { gmGarrisonSpawned coV }
triSimUntil { (gmGarrisonLive coV) >= 4 }
coUnits = []
"coUnits = coUnits + (units _x)" forEach (gmGarrisonGroups coV)
{_x setDamage 1} forEach coUnits
triSimUntil { (gmGarrisonLive coV) == 0 }
gmZoneOnEvent ["counterAttack", {gmEvtCounter = gmEvtCounter + [_this]; coSeen = _this}]
gmZoneOnEvent ["townLost", {gmEvtTownLost = gmEvtTownLost + [_this]; coLost = _this}]
coSq = [coRes, gmFactionSquad [gmResistanceSide, 1, 1], coPos] call GM_fnSpawnSquad
coSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units coSq)
doStop (units coSq)
player setPos coFar
triSimUntil { not ((gmZoneSim coV) select 0) }
triSimUntil { not (gmGarrisonSpawned coO) }
triAssertEq [((gmZone coO) select 3), 6]
triSimUntil { ((gmZone coV) select 2) == gmResistanceSide }

// ---- o2: the fighter walks out; an abstract force leaves the Outpost --------
{deleteVehicle _x} forEach (units coSq)
coFlipT = time
triAssertEq [(count gmCounterAttacks), 1]
triSimUntil { (((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2) != "SCHEDULED" }
triAssertGe [coFlipT + 20, time]
coRow = gmCounterAttacks select 0
triAssertEq [(coRow select 2), "TRAVEL"]
triAssertEq [(coRow select 1), "Outpost"]
triAssertEq [(coRow select 4), 5]
triAssertEq [((gmZone coO) select 3), 1]
triAssertEq [(format ["%1", isNull (coRow select 7)]), "true"]

// ---- o3: arrival in the empty town, the hold, the drain, the revert ---------
triSimUntil { not (isNil "coSeen") }
triAssertEq [(coSeen select 1), "Village"]
triSimUntil { (((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2) != "TRAVEL" }
coS0 = (gmZone coV) select 4
triSimUntil { ((gmZone coV) select 2) == "NEUTRAL" }
triAssertGe [30, ((gmZone coV) select 4)]
triSimUntil { not (isNil "coLost") }
triAssertEq [(coLost select 1), "Village"]
triSimUntil { (count gmCounterAttacks) == 0 }
// the town is administered again with its stored garrison; the force stayed
triAssertEq [((gmZone coV) select 3), 4]
triAssertEq [(format ["%1", gmGarrisonSpawned coV]), "false"]
triAssertEq [((gmZone coO) select 3), 1]
gmSimDiagLog

triEndTest
