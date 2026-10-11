// A usable intro on the played island wins over the base menu even if another
// random candidate has an empty Intro. Stock Malden names intro and intro1.
triSimUntil { triReturnMissionTicks > 0 }
triAssertEq [(typeOf player), "SoldierWB"]
triAssertEq [(triStartRandomCutscene), "OK"]
triAssertEq [(triGameMode), 2]
triAssertEq [(getWorld), "abel"]
triAssertEq [(count (nearestObjects [[7515,5790,0], ["SoldierEB"], 100])), 1]
triSimUntil { triMenuGoodIntro }
triAssert [(isNil "triReturnMissionTicks")]
triSimFrames 60
triAssert [(isNil "triReturnMissionTicks")]
triEndTest
