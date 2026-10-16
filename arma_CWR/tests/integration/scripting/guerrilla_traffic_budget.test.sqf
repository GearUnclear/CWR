// ============================================================================
//  Ambient traffic held to the live-AI budget (round 3 A; OFFSCREEN-CONTRACT.md
//  section 6: budgetLiveBodies 130 is a hard cap).  Boots the SHIPPED
//  Guerrilla.Abel template with the player at the Camp, where the Village
//  (a CITY) and the Outpost (an occupier post) give civ cars and patrols a
//  route:
//    t1  gmTrafficTune runs a pass every second with a spawn on almost every
//        pass (civChance 1, patrolChance 0.3, room under the caps): two
//        ambient spawns land, so the road is live;
//    t2  resistance bodies parked in the far hills (GM_fnSpawnSquad, the
//        branch-comparison lever) bring the census to exactly 130 in one
//        frame, and the kind caps get room again; for 120 s of game time the
//        census never exceeds 130, the traffic pass keeps deciding to spawn
//        and the gate skips it (gmTrafficDiag slot 17), and the budget WARN
//        logs at most once a minute (slot 20);
//    t3  the parked bodies are deleted: traffic resumes (a new ambient spawn,
//        nothing latched).
//  Forced spawns (gmTrafficForceSpawn) are not budget-gated; none are used.
//  The companion roster and the QRF director are parked.  Two script
//  spawners are not held to the budget yet (round 3 A report), so the test
//  parks them to isolate the traffic gate: civilians.sqs stays at its
//  current group count (GM_CIV_MAX_GROUPS 0) and the organic shakedown, two
//  occupier soldiers, never starts (GM_SHK_RANGE 0).
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
tbT0 = time
triSimUntil { time > tbT0 + 6 }
// after the managers have set their tunables (they run once GM_LIB_READY is
// up): the town population and the organic shakedown are script spawners
// that are not held to the budget yet (round 3 A report)
GM_CIV_MAX_GROUPS = 0
GM_SHK_RANGE = 0
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
player setCaptive true
tbRes = gmResistanceSide call GM_fnSideFromString

// ---- t1: a live road --------------------------------------------------------
triAssertEq [(format ["%1", gmTrafficTune ["interval", 1]]), "true"]
triAssertEq [(format ["%1", gmTrafficTune ["civChance", 1]]), "true"]
triAssertEq [(format ["%1", gmTrafficTune ["patrolChance", 0.3]]), "true"]
triAssertEq [(format ["%1", gmTrafficTune ["maxCiv", 8]]), "true"]
triAssertEq [(format ["%1", gmTrafficTune ["maxPatrols", 2]]), "true"]
triAssertEq [(format ["%1", gmTrafficTune ["radius", 900]]), "false"]
triAssertEq [count gmTrafficDiag, 21]
tbSp0 = gmTrafficDiag select 13
triSimUntil { (gmTrafficDiag select 13) >= tbSp0 + 2 }
// let the bubble settle (garrisons, the first cars) before the census is read
tbT1 = time
triSimUntil { time > tbT1 + 15 }

// ---- t2: the census at the cap ------------------------------------------------
tbFar = [4400, 7600, 0]
if ((gmLandmass tbFar) < 0) then {tbFar = [4500, 5200, 0]}
tbFillPos = [(tbFar select 0) + 250, (tbFar select 1) + 250, 0]
tbFill = []
tbL0 = ((gmSimDiag) select 2) select 3; tbFillN = 130 - tbL0; tbF = tbFillN; while {tbFillN > 0} do {tbK = tbFillN; if (tbK > 10) then {tbK = 10}; tbG = [tbRes, gmFactionSquad [gmResistanceSide, 1, tbK], tbFillPos] call GM_fnSpawnSquad; {_x allowDammage false} forEach (units tbG); doStop (units tbG); tbFill = tbFill + [tbG]; tbFillN = tbFillN - tbK}; tbL1 = ((gmSimDiag) select 2) select 3
logInfo format ["TBNOTE census %1 -> %2 with %3 parked resistance bodies in %4 groups", tbL0, tbL1, tbF, count tbFill]
triAssertGe [tbF, 1]
triAssertEq [tbL1, 130]
// room under the kind caps again (the warm-up filled them), so every pass
// decides to spawn and only the budget can say no
triAssertEq [(format ["%1", gmTrafficTune ["maxCiv", 16]]), "true"]
triAssertEq [(format ["%1", gmTrafficTune ["maxPatrols", 4]]), "true"]
tbSk0 = gmTrafficDiag select 17
tbW0 = gmTrafficDiag select 20
tbSp1 = gmTrafficDiag select 13
tbMax = tbL1
tbMaxDiag = gmSimDiag
tbT2 = time
tbPrev = tbL1
triSimUntil { tbL = ((gmSimDiag) select 2) select 3; if (tbL != tbPrev) then {logInfo format ["TBNOTE census %1 -> %2 at %3: %4", tbPrev, tbL, time - tbT2, gmSimDiag]; tbPrev = tbL}; if (tbL > tbMax) then {tbMax = tbL; tbMaxDiag = gmSimDiag}; time > tbT2 + 120 }
tbSk1 = gmTrafficDiag select 17
tbW1 = gmTrafficDiag select 20
tbSp2 = gmTrafficDiag select 13
logInfo format ["TBNOTE watch: max census %1 (%2), skipped %3, warnings %4, spawned %5, diag %6", tbMax, tbMaxDiag, tbSk1 - tbSk0, tbW1 - tbW0, tbSp2 - tbSp1, gmTrafficDiag]
triAssertGe [130, tbMax]
triAssertGe [tbSk1 - tbSk0, 30]
triAssertGe [tbW1 - tbW0, 1]
triAssertGe [3, tbW1 - tbW0]
triAssertGe [gmTrafficDiag select 18, 1]

// ---- t3: room again, the road reopens -------------------------------------------
triAssertEq [(format ["%1", gmTrafficTune ["maxCiv", 24]]), "true"]
{{deleteVehicle _x} forEach (units _x)} forEach tbFill
tbT3 = time
triSimUntil { (((gmSimDiag) select 2) select 3) < 130 }
triSimUntil { (gmTrafficDiag select 13) > tbSp2 }
logInfo format ["TBNOTE resumed after %1 s: census %2, diag %3", time - tbT3, ((gmSimDiag) select 2) select 3, gmTrafficDiag]
gmSimDiagLog

triEndTest
