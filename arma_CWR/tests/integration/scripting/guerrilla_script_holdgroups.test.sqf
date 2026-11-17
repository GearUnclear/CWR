// ============================================================================
//  Spent hold squads are released (decision 8, video 5 finding: capture.sqs
//  kept dead hold groups in GM_HOLD_GRPS and GM_cOld, so the engine's
//  empty-group sweep never released them and they held ids against the
//  63-id register; OFFSCREEN-CONTRACT.md section 6).  Boots the SHIPPED
//  Guerrilla.Abel template:
//    h1  the player walks to the western hills; the Outpost loses its
//        stored garrison and an invulnerable 3-man cell takes it through
//        the real capture meter; capture.sqs gives it a hold squad
//        (GM_HOLD_ZONES / GM_HOLD_GRPS); the cell is deleted with its group;
//    h2  the hold squad is killed (setDamage 1, the wiped-squad case): the
//        resistance group census (gmSimDiag element 3) drops by one, the
//        script drops the row (GM_HOLD_ZONES no longer names the Outpost,
//        no GM_HOLD_GRPS entry, GM_cOld or GM_cGrp is the group) and parks
//        the group in GM_HOLD_SPENT while its dead are still members;
//        [gmsim] emptyGroups= (gmSimDiag element 11) does not keep it: once
//        the dead have left it the script deleteGroups it (GM_HOLD_SPENT
//        empty again, the count back at its baseline) without waiting for
//        the zone tick's 60 s sweep (released= unchanged).
//  The companion roster, the QRF director, the town population and the
//  shakedown are parked, so the group figures move only with this test.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
hgT0 = time
triSimUntil { time > hgT0 + 6 }
GM_CIV_MAX_GROUPS = 0
GM_SHK_RANGE = 0
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true
hgRes = gmResistanceSide call GM_fnSideFromString

// ---- h1: a cell takes the Outpost; capture.sqs leaves a hold squad ------------
hgO = gmZoneIndex "Outpost"
triAssertGe [hgO, 0]
hgOPos = (gmZone hgO) select 8
hgFar = [4500, 5200, 0]
if ((gmLandmass hgFar) < 0) then {hgFar = [7922, 8500, 0]}
player setPos hgFar
triSimUntil { (not ((gmZoneSim hgO) select 0)) and (not (gmGarrisonSpawned hgO)) }
gmZoneSet [hgO, "garrison", 0]
hgCell = [hgRes, gmFactionSquad [gmResistanceSide, 1, 3], hgOPos] call GM_fnSpawnSquad
hgCell setCombatMode "BLUE"
{_x allowDammage false} forEach (units hgCell)
doStop (units hgCell)
hgT1 = time
triSimUntil { (((gmZone hgO) select 2) == gmResistanceSide) or (time > hgT1 + 300) }
triAssertEq [((gmZone hgO) select 2), gmResistanceSide]
hgAt = -1
triSimUntil { hgAt = GM_HOLD_ZONES find "Outpost"; hgAt >= 0 }
hgHold = GM_HOLD_GRPS select hgAt
triSimUntil { ({alive _x} count (units hgHold)) > 0 }
hgName = format ["%1", hgHold]
hgUnits = units hgHold
triAssertGe [count hgUnits, 1]
{deleteVehicle _x} forEach (units hgCell)
deleteGroup hgCell
hgCell = grpNull
hgT2 = time
triSimUntil { time > hgT2 + 6 }
hgD0 = gmSimDiag
hgRes0 = (hgD0 select 3) select 1
hgEmpty0 = (hgD0 select 11) select 0
hgRel0 = (hgD0 select 11) select 1
logInfo format ["HGNOTE hold %1 (%2 men), baseline %3", hgName, count hgUnits, hgD0]
triAssertEq [format ["%1", (GM_HOLD_ZONES find "Outpost") >= 0], "true"]

// ---- h2: the squad is wiped: the row goes, the group is released --------------
{_x setDamage 1} forEach hgUnits
// the test's own handle would pin the group like the script's did
hgHold = grpNull
hgT3 = time
triSimUntil { ((GM_HOLD_ZONES find "Outpost") < 0) or (time > hgT3 + 30) }
triAssertEq [format ["%1", (GM_HOLD_ZONES find "Outpost") < 0], "true"]
hgRef = 0
{if ((format ["%1", _x]) == hgName) then {hgRef = hgRef + 1}} forEach GM_HOLD_GRPS
triAssertEq [hgRef, 0]
triAssertEq [format ["%1", (format ["%1", GM_cOld]) == hgName], "false"]
triAssertEq [format ["%1", (format ["%1", GM_cGrp]) == hgName], "false"]
hgD1 = gmSimDiag
logInfo format ["HGNOTE row dropped after %1 s: %2", time - hgT3, hgD1]
triAssertGe [hgRes0 - ((hgD1 select 3) select 1), 1]
// emptyGroups does not keep it (the script's deleteGroup at once, or the
// sweep after emptyGroupRelease once the dead have left the group)
hgT4 = time
hgSpent = 0
{if ((format ["%1", _x]) == hgName) then {hgSpent = hgSpent + 1}} forEach GM_HOLD_SPENT
logInfo format ["HGNOTE spent list %1 (holds it %2 times), units left %3", GM_HOLD_SPENT, hgSpent, {(format ["%1", group _x]) == hgName} count hgUnits]
triSimUntil { ((((gmSimDiag) select 11) select 0) <= hgEmpty0) or (time > hgT4 + 240) }
hgD2 = gmSimDiag
hgDead = 0
{if (not (isNull _x)) then {hgDead = hgDead + 1}} forEach hgUnits
logInfo format ["HGNOTE released after %1 s (%2 bodies still in the world): %3", time - hgT4, hgDead, hgD2]
gmSimDiagLog
triAssertGe [hgEmpty0, (hgD2 select 11) select 0]
triAssertGe [hgRes0 - ((hgD2 select 3) select 1), 1]
// the script's deleteGroup did it, not the sweep
triAssertEq [(hgD2 select 11) select 1, hgRel0]
hgT5 = time
triSimUntil { time > hgT5 + 3 }
hgSpent = 0
{if ((format ["%1", _x]) == hgName) then {hgSpent = hgSpent + 1}} forEach GM_HOLD_SPENT
triAssertEq [hgSpent, 0]

triEndTest
