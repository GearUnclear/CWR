triSimUntil { gmZoneCount > 0 }
triAssertEq [(triLoadGame "hqvisuals"), "OK"]
triSimFrames 150
triAssert [gmHqIndoors]
triAssertEq [(typeOf gmHqCache), "GMHqCacheIndoor"]
triAssertEq [(typeOf gmHqProp), "GMHqDesk"]
triAssertEq [(count (weaponCargo gmHqCache)), 0]
triAssertEq [(count (magazineCargo gmHqCache)), 0]
triAssertEq [(count (nearestObjects [gmHqCache, ["GMHqDesk"], 50])), 1]
hqOldProp = gmHqProp
hqOldCache = gmHqCache
triAssert [gmHqEstablish ((gmZone (gmZoneIndex "Village")) select 8)]
triSimFrames 10
triAssert [isNull hqOldProp]
triAssert [gmHqCache == hqOldCache]
triAssertEq [(count (nearestObjects [gmHqCache, ["GMHqDesk"], 50])), 1]
triAssertEq [gmStashCount, 1]
triEndTest
