// ============================================================================
//  Enemy Legends, half two: the pin, the tank rule and the defeat latch.
//    guerrilla_legends_place proves WHERE the three commanders are; this lane
//    proves what happens to them:
//      * THE PIN. A commander is DAMove-pinned and nothing else, so under fire
//        he holds his ground and still fights. The stationarity is asserted
//        body-at-t0 against body-at-t0+45, never against the persisted stand
//        with a tight epsilon (createUnit's own free-position search moves him
//        a stride, and that is not drift).
//      * HIS GUARDS ARE LOCAL, NOT PINNED. They carry no disabled-AI flag, so
//        they may move to defend, and they stay near him: the group is given a
//        SENTRY waypoint at his stand at INDEX 1, because CreateSideGroup's own
//        MOVE waypoint sits at index 0 and the arcade FSM starts at 1, so slot
//        0 is never executed either way.
//      * THE TANK RULE. Destroying the hull alone never completes the
//        objective. The commander often survives it (Transport::Destroy deals
//        the crew 0.5..1.0, so he is damage-shielded here to make the branch
//        deterministic), and if he does he fights on foot with his standing
//        get-in order revoked rather than trying to re-board a wreck.
//      * THE LATCH. Killing the commander writes exactly ONE campaign line
//        attributed to his row id, flips his objective to DONE and repaints
//        his marker; and nothing rebuilds him afterwards.
//
//  Waits are CLOCK predicates. triSimFrames pumps AppIdle ticks and advances
//  no clock, while the registry polls at 1 Hz off real deltaT, so a negative
//  assertion expressed in frames can pass without a single boss tick having
//  run. Assertions inside a while body are silent (the harness reads the
//  line's value), so every loop accumulates a diagnostic string instead.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmLegendCount >= 3 }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }

glBoss = []
glI = 0
while {glI < gmLegendCount} do {if (((gmLegendInfo glI) select 2) == 1) then {glBoss = glBoss + [glI]}; glI = glI + 1}
triAssertEq [(count glBoss), 3]
triSimUntil { (not (isNull (gmLegendBody (glBoss select 0)))) and (not (isNull (gmLegendBody (glBoss select 1)))) and (not (isNull (gmLegendBody (glBoss select 2)))) }

// the tank commander, by role. FAIL rather than skip: a branch that quietly
// no-ops is exactly how the headline rule would rot.
// The foot commander is the ELITE one on purpose: he keeps four guards, which
// is what makes the guards-defend-locally assertion mean anything. The sniper
// (one spotter) is left alone, so a live third commander survives the lane.
glTank = -1
glFoot = -1
glI = 0
while {glI < 3} do {if (((gmLegendInfo (glBoss select glI)) select 3) == "Tank Commander") then {glTank = glBoss select glI}; if ((((gmLegendInfo (glBoss select glI)) select 3) == "Commander") and (glFoot < 0)) then {glFoot = glBoss select glI}; glI = glI + 1}
triAssertGe [glTank, 0]
triAssertGe [glFoot, 0]
triAssert [(not (isNull (gmLegendVehicle glTank)))]

// ===========================================================================
//  THE PIN, UNDER FIRE
// ===========================================================================
glPinObj = gmLegendBody glFoot
glPinStand = gmLegendPos glFoot
glPinAt = getPos glPinObj
glPinObj allowDammage false
// his retinue, counted BEFORE the firefight: the guards are deliberately NOT
// shielded, so afterwards only the survivors can be asked where they are
glGuardN0 = count (units (group glPinObj))
triAssertGt [glGuardN0, 1]
// where each of them stands before the shooting starts, so the after picture has
// something to be compared against. Index-matched with glGuardObj.
glGuardObj = []
glGuardAt = []
glI = 0
while {glI < glGuardN0} do {glG = (units (group glPinObj)) select glI; if (glG != glPinObj) then {glGuardObj = glGuardObj + [glG]; glGuardAt = glGuardAt + [getPos glG]}; glI = glI + 1}
triAssertGt [(count glGuardObj), 0]
// 45 m from him, on the line back to the zone he watches: the stand is dry and
// so is the zone, so this lands the fire team on land whichever bearing the
// picker chose. A fixed offset can drop them in the sea on a coastal ring.
glZp = (gmZone (gmZoneIndex ((gmLegendInfo glFoot) select 4))) select 8
glVx = (glZp select 0) - (glPinAt select 0)
glVy = (glZp select 1) - (glPinAt select 1)
glVl = sqrt ((glVx * glVx) + (glVy * glVy))
glFoePos = [(glPinAt select 0) + (45 * glVx / glVl), (glPinAt select 1) + (45 * glVy / glVl), 0]
glFoe = [gmResistanceSide call GM_fnSideFromString, gmFactionTierClass [gmResistanceSide, 1], 4, glFoePos] call GM_fnSpawnGroup
glFoe setBehaviour "COMBAT"
glFoe setCombatMode "RED"
glFoeLeader = leader glFoe
glFoeN = count (units glFoe)
triAssertGt [glFoeN, 0]
glT0 = time
glPinNamed = glPinObj
glTraceNext = time
logInfo format ["PIN_PROBE begin time=%1 pos=%2 move=%3 speed=%4 dir=%5 disabled=%6", time, getPos glPinObj, getMove glPinObj, speed glPinObj, getDir glPinObj, triUnitAIDisabled "glPinNamed"]
triSimUntil { if (time >= glTraceNext) then {logInfo format ["PIN_PROBE time=%1 pos=%2 move=%3 speed=%4 dir=%5 disabled=%6", time, getPos glPinObj, getMove glPinObj, speed glPinObj, getDir glPinObj, triUnitAIDisabled "glPinNamed"]; glTraceNext = time + 1}; time > glT0 + 60 }

// he did not walk away: BODY at t0 against BODY at t0+45
glPinNow = getPos glPinObj
glPinMoved = sqrt ((((glPinNow select 0) - (glPinAt select 0)) * ((glPinNow select 0) - (glPinAt select 0))) + (((glPinNow select 1) - (glPinAt select 1)) * ((glPinNow select 1) - (glPinAt select 1))))
triAssertLt [glPinMoved, 3]
triEndTest
