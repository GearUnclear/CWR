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
//        candidate is inside the audible floor around the player, so no
//        point passes; the template runs occupierForces = 1, so the birth
//        chain re-sources it at once to the next base back whose road
//        passes the gate (OCCUPIER-FORCES.md 3.1, D6: no turn-back), and it
//        is never born at the Outpost;
//    w3  the Outpost has its whole strength again (stored reserve + live
//        garrison + the pending refund = 6).
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

// ---- w2: the player on the Outpost: never born there, re-sourced ----------
// Guerrilla.Abel runs with occupierForces = 1, so the birth chain applies to
// this legacy (director DORMANT) record too (OCCUPIER-FORCES.md 3.1, D6):
// with no unseen point on the Outpost's road it is re-sourced AT ONCE to the
// next base back whose road passes the gate, and never turns back.
wbToTown = ((wbPos select 0) - (wbOPos select 0)) atan2 ((wbPos select 1) - (wbOPos select 1))
player setPos [(wbOPos select 0) + 2, (wbOPos select 1) + 2, 0]
player setDir wbToTown
wbSawLiveHere = false
wbResourced = false
wbT1 = time
triSimUntil { player setDir wbToTown; wbRow = (gmCounterAttacks + [["", "", "NONE"]]) select 0; if (((wbRow select 2) == "LIVE") and ((wbRow select 1) == "Outpost")) then {wbSawLiveHere = true}; if (((wbRow select 1) != "Outpost") and ((wbRow select 1) != "")) then {wbResourced = true}; wbResourced or (time > wbT1 + 40) }
triAssertEq [(format ["%1", wbResourced]), "true"]
triAssertEq [(format ["%1", wbSawLiveHere]), "false"]
wbRow = gmCounterAttacks select 0
triAssertEq [(format ["%1", (wbRow select 1) != "Outpost"]), "true"]

// ---- w3: the Outpost is whole again: its force went home unborn ------------
// the refunded force is a pending deposit (S5) while the player stands on the
// Outpost: stored + live + pending make the post whole again
triSimUntil { (((gmZone wbO) select 3) + (gmGarrisonLive wbO) + ((gmZoneSim wbO) select 13)) == 6 }
triAssertEq [(format ["%1", wbSawLiveHere]), "false"]
gmSimDiagLog

triEndTest
