// ============================================================================
//  The shakedown pair held to the live-AI budget (decision 8 leftover of
//  round 3 A; OFFSCREEN-CONTRACT.md section 6: budgetLiveBodies 130 is a
//  hard cap, and shakedown.sqs asks gmSimAdmit before it spawns its two
//  occupier soldiers, as civilians.sqs and ambient traffic do).  Boots the
//  SHIPPED Guerrilla.Abel template with the player in the Village:
//    s1  the town population comes up around the player, then is held at
//        its group count (GM_CIV_MAX_GROUPS); ambient traffic is held at 0;
//    s2  resistance bodies parked in the far hills bring the census to 130;
//        the forced scene (GM_SHK_FORCE) and the organic roll (chance 1, no
//        cooldown) both want a scene; for 50 s of game time (five director
//        ticks) the census is kept at 130 and no pair is spawned: no scene,
//        no pair group, the force flag still set, and gmShkSkipBudget
//        counts the refused launches;
//    s3  the parked bodies are deleted: the next tick launches the scene with
//        its 2-man pair.
//  The companion roster and the QRF director are parked.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_CIV_READY") }
triSimUntil { not (isNil "GM_SHK_FORCE") }
triSimUntil { not (isNil "gmShkSkipBudget") }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
GM_SHK_RANGE = 0
{deleteVehicle _x} forEach ((units group player) - [player])
sbT0 = time
triSimUntil { time > sbT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true
sbRes = gmResistanceSide call GM_fnSideFromString
triAssertEq [(format ["%1", gmTrafficTune ["maxCiv", 0]]), "true"]
triAssertEq [(format ["%1", gmTrafficTune ["maxPatrols", 0]]), "true"]

// ---- s1: the Village populated, then held ------------------------------------
sbV = gmZoneIndex "Village"
triAssertGe [sbV, 0]
sbVPos = (gmZone sbV) select 8
player setPos [sbVPos select 0, sbVPos select 1, 0]
triSimUntil { (count GM_CIV_GROUPS) > 0 }
GM_CIV_MAX_GROUPS = count GM_CIV_GROUPS
sbT1 = time
triSimUntil { time > sbT1 + 10 }

// ---- s2: the census at the cap; both scene paths want a pair ------------------
sbFar = [4400, 7600, 0]
if ((gmLandmass sbFar) < 0) then {sbFar = [4500, 5200, 0]}
sbFillPos = [(sbFar select 0) + 250, (sbFar select 1) + 250, 0]
sbFill = []
sbL0 = ((gmSimDiag) select 2) select 3; sbFillN = 130 - sbL0; sbF = sbFillN; while {sbFillN > 0} do {sbK = sbFillN; if (sbK > 10) then {sbK = 10}; sbG = [sbRes, gmFactionSquad [gmResistanceSide, 1, sbK], sbFillPos] call GM_fnSpawnSquad; {_x allowDammage false} forEach (units sbG); doStop (units sbG); sbFill = sbFill + [sbG]; sbFillN = sbFillN - sbK}; sbL1 = ((gmSimDiag) select 2) select 3
logInfo format ["SBNOTE census %1 -> %2 with %3 parked resistance bodies in %4 groups", sbL0, sbL1, sbF, count sbFill]
triAssertGe [sbF, 1]
triAssertGe [sbL1, 130]
triAssertGe [gmSimAdmit [gmOccupierSide, 2, 1], 1]
sbSk0 = gmShkSkipBudget
GM_SHK_COOLDOWN = 0
GM_SHK_CHANCE = 1
GM_SHK_RANGE = 500
GM_SHK_FORCE = true
sbActive = false
sbTop = 0
sbMin = sbL1
sbT2 = time
triSimUntil { sbL = ((gmSimDiag) select 2) select 3; if (sbL < sbMin) then {sbMin = sbL}; if (sbL < 130) then {sbK = 130 - sbL; if (sbK > 10) then {sbK = 10}; sbG = [sbRes, gmFactionSquad [gmResistanceSide, 1, sbK], sbFillPos] call GM_fnSpawnSquad; {_x allowDammage false} forEach (units sbG); doStop (units sbG); sbFill = sbFill + [sbG]; sbTop = sbTop + 1}; if (GM_SHK_ACTIVE) then {sbActive = true}; time > sbT2 + 50 }
sbSk1 = gmShkSkipBudget
logInfo format ["SBNOTE watch: min census %1, top-ups %2, refused %3, diag %4", sbMin, sbTop, sbSk1 - sbSk0, gmSimDiag]
triAssertEq [format ["%1", sbActive], "false"]
triAssertEq [format ["%1", GM_SHK_ACTIVE], "false"]
triAssertEq [format ["%1", isNull GM_SHK_GRP], "true"]
triAssertEq [format ["%1", GM_SHK_FORCE], "true"]
triAssertGe [sbSk1 - sbSk0, 3]

// ---- s3: room again: the scene launches with its pair -------------------------
{{deleteVehicle _x} forEach (units _x)} forEach sbFill
{deleteGroup _x} forEach sbFill
sbFill = []
sbG = grpNull
sbT3 = time
triSimUntil { (((gmSimDiag) select 2) select 3) < 128 }
sbTk = gmShkTicks
sbSk2 = gmShkSkipBudget
triSimUntil { if ((gmShkTicks != sbTk) or (gmShkSkipBudget != sbSk2)) then {logInfo format ["SBNOTE tick %1 at %2 s: refused %3, force %4, census %5, civ groups %6", gmShkTicks, time - sbT3, gmShkSkipBudget, GM_SHK_FORCE, ((gmSimDiag) select 2) select 3, count GM_CIV_GROUPS]; sbTk = gmShkTicks; sbSk2 = gmShkSkipBudget}; GM_SHK_ACTIVE or (time > sbT3 + 90) }
logInfo format ["SBNOTE launched after %1 s: census %2, refused %3 since the watch", time - sbT3, ((gmSimDiag) select 2) select 3, gmShkSkipBudget - sbSk1]
triAssertEq [format ["%1", GM_SHK_ACTIVE], "true"]
triAssertEq [(count units GM_SHK_GRP), 2]
triAssertEq [format ["%1", GM_SHK_FORCE], "false"]
gmSimDiagLog

triEndTest
