triSimUntil { not (isNil "HT_READY") }
triSimUntil { HT_READY }
[player, player, HT_ACTIONS select 0] exec "human/action.sqs"
triSimUntil { HT_OPEN }
htRoadIndex = 0
while { ((HT_CASES select htRoadIndex) select 0) != "traffic_patrol" } do {htRoadIndex = htRoadIndex + 1}
HT_INDEX = htRoadIndex
// Wrong position: no hidden ownership change or result credit.
[player, player, HT_ACTIONS select 6] exec "human/action.sqs"
triSimFrames 30
triAssertEq [(gmZone SC_CAMP) select GM_Z_OWNER, gmResistanceSide]
triAssertEq [count HT_SETUP_HISTORY, 0]
player setPos ((gmZone SC_VILL) select GM_Z_POS)
[player, player, HT_ACTIONS select 6] exec "human/action.sqs"
triSimUntil { (count HT_SETUP_HISTORY) > 0 }
triAssert [not (isNull HT_vehicle)]
triAssertEq [(gmTrafficInfo HT_vehicle) select 0, "patrol"]
triAssertEq [(gmTrafficInfo HT_vehicle) select 1, SC_OUT]
triAssertEq [(gmTrafficInfo HT_vehicle) select 2, SC_CAMP]
triAssertEq [HT_RESULTS select htRoadIndex, "UNTESTED"]
triAssertEq [count (HT_EVIDENCE select htRoadIndex), 0]
htRoadPos = getPos HT_vehicle
triSimUntil { ((getPos HT_vehicle) distance htRoadPos) > 15 }
// Duplicate setup cannot mint another patrol.
[player, player, HT_ACTIONS select 6] exec "human/action.sqs"
triSimFrames 30
triAssertEq [count HT_SETUP_HISTORY, 1]
// Convoy uses the same production service, its real truck and escort.
htRoadIndex = htRoadIndex + 1
HT_INDEX = htRoadIndex
triAssertEq [(HT_CASES select HT_INDEX) select 0, "traffic_convoy"]
[player, player, HT_ACTIONS select 6] exec "human/action.sqs"
triSimUntil { (count HT_SETUP_HISTORY) == 2 }
triAssert [not (isNull HT_vehicle)]
triAssertEq [(gmTrafficInfo HT_vehicle) select 0, "convoy"]
triAssert [not (isNull (gmTrafficEscort HT_vehicle))]
triAssertEq [HT_RESULTS select HT_INDEX, "UNTESTED"]
triEndTest
