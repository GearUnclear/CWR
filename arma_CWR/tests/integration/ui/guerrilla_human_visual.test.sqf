triSimUntil { not (isNil "HT_READY") }
triSimUntil { HT_READY }
triBindAction ["NextAction", 48]
triSendKey 48
triSimFrames 10
triAssertIncludes [(triActionMenuText), "HUMAN TESTS"]
triScreenshot "01_human_entry"
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_OPEN }
triSendKey 48
triSimFrames 10
triAssertIncludes [(triActionMenuText), "begin selected case"]
triScreenshot "02_human_browser"
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_ACTIVE }
triSendKey 48
triSimFrames 10
triAssertIncludes [(triActionMenuText), "check step"]
triAssertIncludes [(triActionMenuText), "BLOCKED"]
triScreenshot "03_human_step"
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimFrames 30
triAssertEq [HT_STEP, 0]
triScreenshot "04_human_not_yet"
triEndTest
