// ============================================================================
//  Occupier counter-attack, WATCHED BASE (the user's rule of 2026-10-13: a
//  counter-attack never pops in where the player can see it; native
//  CounterAttack service, guerrilla-mode/OFFSCREEN-CONTRACT.md section 9).
//  Boots the SHIPPED Guerrilla.Abel template on the full Remaster:
//    w1  as in the off-screen test, Houdan rises remotely with the player
//        2.5 km away, and its answer leaves the Outpost's stored reserve as
//        an ABSTRACT force on the road (abstract speed tuned slow, so it is
//        still at its base);
//    w2  the player walks onto the Outpost itself, looking down the road
//        toward Houdan: the town is in his bubble, so the force must be
//        born, but it has barely left its base and a travelling force is
//        only re-born BEHIND its progress point (never ahead of it): every
//        candidate is inside the audible floor around the player, so it is
//        deferred, it never appears, its arrival clock stops and it does
//        nothing to the town;
//    w3  after counterAttackBirthGiveUp (tuned to 45 s) it turns back: the
//        record is finished and the Outpost has its whole strength again
//        (stored reserve + live garrison = 6).
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
wbT0 = time
triSimUntil { time > wbT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

wbRes = gmResistanceSide call GM_fnSideFromString
wbV = gmZoneIndex "Village"
wbO = gmZoneIndex "Outpost"
triAssertGe [wbV, 0]
triAssertGe [wbO, 0]
wbPos = (gmZone wbV) select 8
wbOPos = (gmZone wbO) select 8
wbFar = [7922, 8500, 0]
triAssertEq [(format ["%1", gmCounterAttackTune ["delayMin", 5]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["delayMax", 10]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["townsTaken", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["birthGiveUp", 45]]), "true"]

// ---- w1: Houdan rises remotely; an abstract force leaves the Outpost --------
triSimUntil { gmGarrisonSpawned wbV }
triSimUntil { (gmGarrisonLive wbV) >= 4 }
wbUnits = []
"wbUnits = wbUnits + (units _x)" forEach (gmGarrisonGroups wbV)
{_x setDamage 1} forEach wbUnits
triSimUntil { (gmGarrisonLive wbV) == 0 }
wbSq = [wbRes, gmFactionSquad [gmResistanceSide, 1, 1], wbPos] call GM_fnSpawnSquad
wbSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units wbSq)
doStop (units wbSq)
player setPos wbFar
triSimUntil { not ((gmZoneSim wbV) select 0) }
triSimUntil { not (gmGarrisonSpawned wbO) }
triAssertEq [((gmZone wbO) select 3), 6]
triSimUntil { ((gmZone wbV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units wbSq)
triSimUntil { (((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2) == "TRAVEL" }
wbRow = gmCounterAttacks select 0
triAssertEq [(wbRow select 1), "Outpost"]

// ---- w2: the player on the Outpost: never born there, deferred ------------
wbToTown = ((wbPos select 0) - (wbOPos select 0)) atan2 ((wbPos select 1) - (wbOPos select 1))
player setPos [(wbOPos select 0) + 2, (wbOPos select 1) + 2, 0]
player setDir wbToTown
wbSawLive = false
wbOccSeen = false
wbT1 = time
triSimUntil { player setDir wbToTown; if ((((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2) == "LIVE") then {wbSawLive = true}; if (((gmZoneSim wbV) select 2) > 0) then {wbOccSeen = true}; time > wbT1 + 5 }
wbClock = ((gmCounterAttacks select 0) select 3)
wbT2 = time
triSimUntil { player setDir wbToTown; if ((((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2) == "LIVE") then {wbSawLive = true}; if (((gmZoneSim wbV) select 2) > 0) then {wbOccSeen = true}; time > wbT2 + 20 }
wbRow = gmCounterAttacks select 0
triAssertEq [(wbRow select 2), "TRAVEL"]
triAssertEq [(wbRow select 3), wbClock]
triAssertEq [(format ["%1", wbSawLive]), "false"]
// nothing done to Houdan meanwhile: it is still held, unless a LIVE
// occupier unit stood in it (a passing traffic patrol drains a held town by
// design, OFFSCREEN-CONTRACT section 9 "Accidental retakes"; the unborn force
// is no presence at all)
triAssertEq [(format ["%1", (((gmZone wbV) select 2) == gmResistanceSide) or wbOccSeen]), "true"]

// ---- w3: no unseen road: it turns back, the Outpost is whole again ---------
triSimUntil { player setDir wbToTown; if ((((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2) == "LIVE") then {wbSawLive = true}; if (((gmZoneSim wbV) select 2) > 0) then {wbOccSeen = true}; (count gmCounterAttacks) == 0 }
triAssertEq [(format ["%1", wbSawLive]), "false"]
triSimUntil { (((gmZone wbO) select 3) + (gmGarrisonLive wbO)) == 6 }
triAssertEq [(format ["%1", (((gmZone wbV) select 2) == gmResistanceSide) or wbOccSeen]), "true"]
gmSimDiagLog

triEndTest
