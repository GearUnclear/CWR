triSimUntil { not (isNil "HT_READY") }
triSimUntil { HT_READY }
triAssert [not HT_OPEN]
triAssertEq [(triLoadGame "human_suite"), "OK"]
triSimUntil { HT_LOADS > HT_STARTLOAD }
triAssert [HT_OPEN]
triAssert [HT_ACTIVE]
triAssertEq [HT_STEP, 1]
triAssertEq [HT_INDEX, htSaveIndex]
triAssertEq [count HT_ACTIONS, 5]
triAssertEq [format ["%1", HT_EVIDENCE select HT_INDEX], htSavedEvidence]
triAssertEq [gmResources, htSavedR]
triAssertEq [gmManpower, htSavedHR]
triAssertEq [HT_RESULTS select HT_INDEX, "IN_PROGRESS"]
// Restored step accepts the actual load, but still requires a human click.
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_STEP == 2 }
triAssertEq [count (HT_EVIDENCE select HT_INDEX), 2]
// Stop rather than claim the rest of the manual play-through was performed.
[player, player, HT_ACTIONS select 4] exec "human/action.sqs"
triSimUntil { not HT_ACTIVE }
triAssertEq [HT_RESULTS select HT_INDEX, "INCOMPLETE"]
triAssertEq [count HT_ACTIONS, 7]
triEndTest
