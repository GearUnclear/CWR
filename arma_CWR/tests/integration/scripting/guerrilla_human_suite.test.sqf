// Exercises the suite's actual action dispatcher and production recruitment.
// Harness-driven step confirmations validate plumbing, NOT human acceptance.
triSimUntil { not (isNil "HT_READY") }
triSimUntil { HT_READY }
triSimUntil { (count GM_COMP_OBJ) > 0 }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triAssertEq [count HT_CASES, 43]
triAssertEq [count HT_RESULTS, count HT_CASES]
triAssertEq [count HT_ACTIONS, 1]
triAssert [not HT_OPEN]
triAssert [not SC_STAGED]

// Compile/execute each eligible case's production-state observations.
// Baseline helpers may only write HT_*; this is also source-audited.
htR = gmResources
htH = gmManpower
htGear = format ["%1", GM_GEAR_COUNT]
htI = 0
while {htI < count HT_CASES} do {htRow = HT_CASES select htI; if ([] call (htRow select 4)) then {[] call (htRow select 5); {htProbe = [] call (_x select 2)} forEach (htRow select 6)}; htI = htI + 1}
triAssertEq [gmResources, htR]
triAssertEq [gmManpower, htH]
triAssertEq [format ["%1", GM_GEAR_COUNT], htGear]

[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_OPEN }
triAssertEq [count HT_ACTIONS, 7]
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_ACTIVE }
triAssertEq [HT_STEP, 0]
// No movement: CHECK must not advance or manufacture a pass.
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimFrames 60
triAssertEq [HT_STEP, 0]
triAssertEq [count (HT_EVIDENCE select 0), 0]
triAssertEq [HT_RESULTS select 0, "IN_PROGRESS"]
// Simulated player movement for the runner test only; suite never teleports.
htP = getPos player
player setPos [(htP select 0) + 25, htP select 1, 0]
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_STEP == 1 }
triAssertEq [count (HT_EVIDENCE select 0), 1]
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { not HT_ACTIVE }
triAssertEq [HT_RESULTS select 0, "PASS (HUMAN)"]
triAssertEq [count (HT_EVIDENCE select 0), 2]
triAssertEq [count HT_HISTORY, 1]

// Start a case that needs no prepared incident, then exercise explicit FAIL.
HT_INDEX = 0
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_ACTIVE }
[player, player, HT_ACTIONS select 2] exec "human/action.sqs"
triSimUntil { not HT_ACTIVE }
triAssertEq [HT_RESULTS select 0, "FAIL"]
triAssertEq [count HT_HISTORY, 2]
// Invalid/stale action IDs cannot become a request.
[player, player, -12345] exec "human/action.sqs"
triSimFrames 30
triAssertEq [HT_REQUEST, ""]

// Main-mode recruitment: human suite only observes it.
player setPos gmCampPos
triSimUntil { gmRecruitActive }
htI = 0
while { ((HT_CASES select htI) select 0) != "recruit" } do { htI = htI + 1 }
HT_INDEX = htI
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_ACTIVE }
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimFrames 30
triAssertEq [HT_STEP, 0]
[player, player, gmActRecruit] exec "\gmcore\scripts\recruit_action.sqs"
triSimUntil { (count units (group player)) > count HT_units }
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_STEP == 1 }
[player, player, gmActSpec] exec "\gmcore\scripts\recruit_action.sqs"
triSimUntil { (count units (group player)) >= ((count HT_units) + 2) }
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { not HT_ACTIVE }
triAssertEq [HT_RESULTS select htI, "PASS (HUMAN)"]

// Staged requests are rejected even if an external harness queues one.
SC_QUEUE = [1]
triSimUntil { (count SC_QUEUE) == 0 }
triAssert [not SC_STAGED]
triAssert [not SC_BUSY]
// BLOCKED and stopped attempts are separate from successful acceptance.
HT_INDEX = 0
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_ACTIVE }
[player, player, HT_ACTIONS select 3] exec "human/action.sqs"
triSimUntil { not HT_ACTIVE }
triAssertEq [HT_RESULTS select 0, "BLOCKED"]
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_ACTIVE }
[player, player, HT_ACTIONS select 4] exec "human/action.sqs"
triSimUntil { not HT_ACTIVE }
triAssertEq [HT_RESULTS select 0, "INCOMPLETE"]
// Close and ensure the original menu is restored.
[player, player, HT_ACTIONS select 5] exec "human/action.sqs"
triSimUntil { not HT_OPEN }
triAssertEq [count HT_ACTIONS, 1]
triAssertEq [count SC_ACTS, count SC_CH_IDS]
// Once a staged chapter has run, this session cannot be represented as
// human acceptance. Use the readout chapter to avoid changing campaign facts.
SC_AUTO = true
SC_QUEUE = [10]
triSimUntil { SC_LASTDONE == 10 }
triAssert [SC_STAGED]
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimFrames 30
triAssert [not HT_OPEN]
triEndTest
