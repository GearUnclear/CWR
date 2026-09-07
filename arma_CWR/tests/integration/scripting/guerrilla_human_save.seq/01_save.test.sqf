triSimUntil { not (isNil "HT_READY") }
triSimUntil { HT_READY }
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_OPEN }
htSaveIndex = 0
while { ((HT_CASES select htSaveIndex) select 0) != "persistence" } do {htSaveIndex = htSaveIndex + 1}
HT_INDEX = htSaveIndex
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_ACTIVE }
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_STEP == 1 }
triAssertEq [count (HT_EVIDENCE select HT_INDEX), 1]
// A second CHECK without a load cannot pass the persistence gate.
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimFrames 30
triAssertEq [HT_STEP, 1]
// Real Save action dispatch, followed by a named snapshot for isolated
// two-process harness reload (triLoadGame locates named test snapshots).
[player, player, GM_pSaveAct] exec "\gmcore\scripts\campaign.sqs"
triSimFrames 60
htSavedR = gmResources
htSavedHR = gmManpower
htSavedEvidence = format ["%1", HT_EVIDENCE select HT_INDEX]
triAssertEq [(triSaveGame "human_suite"), "OK"]
triEndTest
