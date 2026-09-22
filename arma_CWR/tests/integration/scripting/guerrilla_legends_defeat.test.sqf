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
triSimUntil { time > glT0 + 60 }

// he did not walk away: BODY at t0 against BODY at t0+45
glPinNow = getPos glPinObj
glPinMoved = sqrt ((((glPinNow select 0) - (glPinAt select 0)) * ((glPinNow select 0) - (glPinAt select 0))) + (((glPinNow select 1) - (glPinAt select 1)) * ((glPinNow select 1) - (glPinAt select 1))))
triAssertLt [glPinMoved, 3]
// ... and he is still DAMove and only DAMove
glPinNamed = glPinObj
triAssertEq [(triUnitAIDisabled "glPinNamed"), 2]

// he still fights: acquisition and fire are untouched by the pin, so after 45 s
// of contact he knows about the squad shooting at him (or has already hurt it)
glEngaged = 0
glEvidence = ""
if ((glPinObj knowsAbout glFoeLeader) > 0) then {glEngaged = glEngaged + 1; glEvidence = glEvidence + "knows "}
glI = 0
while {glI < (count (units glFoe))} do {if ((getDammage ((units glFoe) select glI)) > 0) then {glEngaged = glEngaged + 1; glEvidence = glEvidence + "hurt "}; if (not (alive ((units glFoe) select glI))) then {glEngaged = glEngaged + 1; glEvidence = glEvidence + "killed "}; glI = glI + 1}
if ((count (units glFoe)) < glFoeN) then {glEngaged = glEngaged + 1; glEvidence = glEvidence + "gone "}
triAssertGt [glEngaged, 0]

// his guards defend LOCALLY. Not pinned (they have to be able to fight), but
// the group's own posture keeps them on his stand rather than walking to the
// waypoint CreateSideGroup left at the map origin.
glBad = ""
glGuards = units (group glPinObj)
glI = 0
while {glI < (count glGuards)} do {glG = glGuards select glI; if (alive glG) then {glGp = getPos glG; glGd = sqrt ((((glGp select 0) - (glPinStand select 0)) * ((glGp select 0) - (glPinStand select 0))) + (((glGp select 1) - (glPinStand select 1)) * ((glGp select 1) - (glPinStand select 1)))); if (glGd > 60) then {glBad = glBad + (format ["[%1]%2m ", glI, glGd])}}; glI = glI + 1}
triAssertEq [glBad, ""]

// ... and that near-him assertion means something only because they COULD have
// left. A bodyguard carries no disabled-AI flag at all (the boss alone is
// DAMove pinned, and PinBossActors gates the crew loop on row.crewed), which is
// the mirror of the boss assertion above and the thing that would silently
// break if the pin were ever widened back over guards[]. glGuardMoved is the
// same statement measured rather than declared, reported but not asserted: a
// guard is free to decide his own ground is the best cover on the field.
glGuardNamed = objNull
glGuardMoved = 0
glI = 0
while {glI < (count glGuardObj)} do {glG = glGuardObj select glI; if (alive glG) then {if (isNull glGuardNamed) then {glGuardNamed = glG}; glGp = getPos glG; glG0 = glGuardAt select glI; glGd = sqrt ((((glGp select 0) - (glG0 select 0)) * ((glGp select 0) - (glG0 select 0))) + (((glGp select 1) - (glG0 select 1)) * ((glGp select 1) - (glG0 select 1)))); if (glGd > glGuardMoved) then {glGuardMoved = glGd}}; glI = glI + 1}
triAssert [(not (isNull glGuardNamed))]
triAssertEq [(triUnitAIDisabled "glGuardNamed"), 0]

{deleteVehicle _x} forEach (units glFoe)
glPinObj allowDammage true

// ===========================================================================
//  THE TANK RULE: the hull is not the man
// ===========================================================================
glTankObj = gmLegendBody glTank
glTankId = (gmLegendInfo glTank) select 0
glTankName = (gmLegendInfo glTank) select 1
glTankRole = (gmLegendInfo glTank) select 3
// The crew take 0.5..1.0 of the overkill when the hull dies, so shielding him
// is what makes this branch a rule and not a coin toss. The RULE is that the
// hull's death alone never completes the objective, whether he lives or not.
glTankObj allowDammage false
(gmLegendVehicle glTank) setDammage 1
triSimUntil { isNull (gmLegendVehicle glTank) }
glT0 = time
triSimUntil { time > glT0 + 20 }
triAssertEq [(gmLegendDefeated glTank), false]
triAssertEq [(gmJournalObjectiveState ("legend_" + glTankId)), "ACTIVE"]
triAssertEq [((gmLegendMarker glTank) select 2), "ColorRed"]
triAssertEq [((gmLegendMarker glTank) select 3), (glTankName + ", " + glTankRole)]
triAssert [(alive glTankObj)]
// the standing get-in order is revoked when the hull dies, so he fights on
// foot instead of trying to re-board a wreck
triAssertEq [((vehicle glTankObj) == glTankObj), true]
glT0 = time
triSimUntil { time > glT0 + 15 }
triAssertEq [((vehicle glTankObj) == glTankObj), true]
triAssertEq [(gmLegendDefeated glTank), false]

// ... and the commander himself still closes it
glJ0 = gmJournalCount
glTankObj allowDammage true
glTankObj setDammage 1
triSimUntil { gmLegendDefeated glTank }
triAssertEq [(gmJournalObjectiveState ("legend_" + glTankId)), "DONE"]
triAssertEq [((gmLegendMarker glTank) select 2), "ColorGreen"]
triAssertEq [((gmLegendMarker glTank) select 3), (glTankName + " (defeated)")]
triAssertEq [((gmLegendMarker glTank) select 0), ("gmLegend_" + glTankId)]

// ===========================================================================
//  ONE LINE, ONCE - AND NO RESPAWN
// ===========================================================================
glFootId = (gmLegendInfo glFoot) select 0
glFootBefore = 0
glJi = 0
while {glJi < gmJournalCount} do {if ((gmJournalEntryChar glJi) == glFootId) then {glFootBefore = glFootBefore + 1}; glJi = glJi + 1}
glCountBefore = gmLegendCount
(gmLegendBody glFoot) setDammage 1
triSimUntil { gmLegendDefeated glFoot }
glFootAfter = 0
glJi = 0
while {glJi < gmJournalCount} do {if ((gmJournalEntryChar glJi) == glFootId) then {glFootAfter = glFootAfter + 1}; glJi = glJi + 1}
triAssertEq [glFootAfter, glFootBefore + 1]
triAssertEq [(gmJournalObjectiveState ("legend_" + glFootId)), "DONE"]

// sixty more seconds of the 1 Hz poll: not one further line, not one further
// row, and nothing rebuilt. row.spawned is persisted and latched BEFORE the
// actors are created, so a dead commander is never rebuilt.
glT0 = time
triSimUntil { time > glT0 + 60 }
glFootLater = 0
glJi = 0
while {glJi < gmJournalCount} do {if ((gmJournalEntryChar glJi) == glFootId) then {glFootLater = glFootLater + 1}; glJi = glJi + 1}
triAssertEq [glFootLater, glFootAfter]
triAssertEq [gmLegendCount, glCountBefore]
triAssertEq [(gmLegendDefeated glFoot), true]
triAssertEq [(gmLegendDefeated glTank), true]
triAssert [(isNull (gmLegendBody glFoot)) or (not (alive (gmLegendBody glFoot)))]
triAssertEq [((gmLegendMarker glFoot) select 2), "ColorGreen"]
// the third commander is untouched by either death
triAssertEq [(gmLegendDefeated (glBoss select 0)), ((glBoss select 0) == glFoot) or ((glBoss select 0) == glTank)]
glLive = -1
glI = 0
while {glI < 3} do {if (not (gmLegendDefeated (glBoss select glI))) then {glLive = glBoss select glI}; glI = glI + 1}
triAssertGe [glLive, 0]
triAssertEq [(gmJournalObjectiveState ("legend_" + ((gmLegendInfo glLive) select 0))), "ACTIVE"]
triAssertEq [((gmLegendMarker glLive) select 2), "ColorRed"]

triEndTest
