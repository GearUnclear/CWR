// ============================================================================
//  Empty groups and the group budget (video 4 finding; OFFSCREEN-CONTRACT.md
//  section 6).  The video 4 scenario deleteVehicle'd its squads but not their
//  groups, and the resistance group census climbed from 1 to 14 with 11
//  bodies: the 40-group side budget counted empty groups, and the engine has
//  no empty-group sweep of its own.  Boots the SHIPPED Guerrilla.Abel
//  template with the player at the Camp:
//    t1  20 resistance groups of two are created in the far hills: the
//        census counts them (gmSimDiag element 3);
//    t2  their units are deleteVehicle'd, the groups are not deleted: the
//        budget census (gmSimDiag element 3, and gmSimAdmit, which reads the
//        spawners' census) no longer counts them, and the [gmsim] figure
//        emptyGroups= (gmSimDiag element 11) reports them;
//    t3  the script drops its handles to 19 of them and keeps one: after
//        90 s of game time the zone tick has released the 19 (no members,
//        no links, idle over emptyGroupRelease 60 s), [gmsim] released=
//        counts them, the kept group is still there and takes a new unit (a
//        group a script holds is a pending spawn, never released), and the
//        player's group survives.
//  The companion roster and the QRF director are parked; the town
//  population is held at its current group count (GM_CIV_MAX_GROUPS 0) so
//  the resistance and empty-group figures move only with this test's groups.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
ceT0 = time
triSimUntil { time > ceT0 + 6 }
GM_CIV_MAX_GROUPS = 0
GM_SHK_RANGE = 0
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true
ceRes = gmResistanceSide call GM_fnSideFromString
cePlayerGroup = format ["%1", group player]

ceFar = [4400, 7600, 0]
if ((gmLandmass ceFar) < 0) then {ceFar = [4500, 5200, 0]}
ceClasses = gmFactionSquad [gmResistanceSide, 1, 2]
triAssertEq [count ceClasses, 2]

ceD0 = gmSimDiag
ceRes0 = (ceD0 select 3) select 1
ceEmpty0 = (ceD0 select 11) select 0
ceRel0 = (ceD0 select 11) select 1
logInfo format ["CENOTE baseline: %1", ceD0]

// ---- t1: 20 groups of two, counted ------------------------------------------
ceGroups = []
ceNull = 0
ceI = 0
while {ceI < 20} do {ceG = createGroup ceRes; if (isNull ceG) then {ceNull = ceNull + 1} else {ceP = [(ceFar select 0) + 30 * (ceI mod 5), (ceFar select 1) + 30 * (ceI / 5 - ((ceI / 5) mod 1)), 0]; {_x createUnit [ceP, ceG, "", 0.5, "PRIVATE"]} forEach ceClasses; {_x allowDammage false} forEach (units ceG); doStop (units ceG); ceGroups = ceGroups + [ceG]}; ceI = ceI + 1}
ceG = grpNull
triAssertEq [ceNull, 0]
triAssertEq [count ceGroups, 20]
ceUnits = 0
{ceUnits = ceUnits + (count units _x)} forEach ceGroups
triAssertEq [ceUnits, 40]
triSimFrames 5
ceD1 = gmSimDiag
ceRes1 = (ceD1 select 3) select 1
logInfo format ["CENOTE spawned: %1", ceD1]
triAssertGe [ceRes1, ceRes0 + 20]

// ---- t2: units deleted, groups kept: outside the budget, reported -----------
{{deleteVehicle _x} forEach (units _x)} forEach ceGroups
triSimFrames 5
ceD2 = gmSimDiag
ceRes2 = (ceD2 select 3) select 1
ceEmpty2 = (ceD2 select 11) select 0
logInfo format ["CENOTE deleted: %1", ceD2]
ceUnits = 0
{ceUnits = ceUnits + (count units _x)} forEach ceGroups
triAssertEq [ceUnits, 0]
triAssertEq [ceRes1 - ceRes2, 20]
triAssertGe [ceEmpty2, ceEmpty0 + 20]
// the spawners' census agrees: with the 20 empty groups counted the
// resistance side would be 20 past this admission
triAssertEq [gmSimAdmit [gmResistanceSide, 0, 40 - ceRes2], 0]
// one [gmsim] line now: emptyGroups= carries them (the run's log)
gmSimDiagLog

// ---- t3: dropped handles are released after the hold, a kept one is not -----
ceKeep = ceGroups select 0
ceGroups = []
ceT1 = time
triSimUntil { time > ceT1 + 90 }
ceD3 = gmSimDiag
ceEmpty3 = (ceD3 select 11) select 0
ceRel3 = (ceD3 select 11) select 1
logInfo format ["CENOTE after 90 s: %1", ceD3]
gmSimDiagLog
triAssertGe [ceRel3, ceRel0 + 19]
triAssertGe [ceEmpty2 - ceEmpty3, 19]
// the kept group: still a group, empty, and a spawn into it lands
triAssertEq [format ["%1", isNull ceKeep], "false"]
triAssertEq [count units ceKeep, 0]
(ceClasses select 0) createUnit [ceFar, ceKeep, "", 0.5, "PRIVATE"]
triAssertEq [count units ceKeep, 1]
// the player's group survives the sweep
triAssertEq [format ["%1", isNull (group player)], "false"]
triAssertEq [format ["%1", group player], cePlayerGroup]
triAssertEq [format ["%1", alive player], "true"]
{deleteVehicle _x} forEach (units ceKeep)
deleteGroup ceKeep

triEndTest
