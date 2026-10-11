// No valid island or base intro exists. Cleanup must still initialize intro
// mode and destroy the old mission's player and looping init.sqs script.
triSimUntil { triReturnMissionTicks > 0 }
triAssertEq [(typeOf player), "SoldierWB"]
triAssertEq [(triStartRandomCutscene), "OK"]
triAssertEq [(triGameMode), 2]
triAssert [(isNull player)]
triAssert [(isNil "triReturnMissionTicks")]
triAssertEq [(gmZoneCount), 0]
triSimFrames 60
triAssert [(isNil "triReturnMissionTicks")]
triAssert [(isNull player)]
triEndTest
