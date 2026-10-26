triSimUntil { gmZoneCount > 0 }
triAssertEq [(triLoadGame "legacyhq"), "OK"]
triSimFrames 1
triAssertEq [(typeOf gmHqCache), "WeaponHolder"]
triSimFrames 150
triAssert [gmHqIndoors]
triAssertEq [(typeOf gmHqCache), "GMHqCacheIndoor"]
triAssertEq [(typeOf gmHqProp), "GMHqDesk"]
triAssertEq [(count (weaponCargo gmHqCache)), 2]
triAssertEq [(count (magazineCargo gmHqCache)), 3]
triAssertEq [gmHqMoveCount, 0]
triAssertEq [gmStashCount, 1]
triAssertEq [(triSaveGame "upgradedhq"), "OK"]
triEndTest
