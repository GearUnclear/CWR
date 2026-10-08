triSimUntil { gmZoneCount > 0 }
triAssertEq [(triLoadGame "hqvisuals"), "OK"]
triSimFrames 10
triAssert [not gmHqIndoors]
triAssertEq [(typeOf gmHqCache), "GMHqCacheOutdoor"]
triAssertEq [(typeOf gmHqProp), "GMHqTent"]
triAssertEq [(count (weaponCargo gmHqCache)), 2]
triAssertEq [(count (magazineCargo gmHqCache)), 3]
hqOldProp = gmHqProp
hqOldCache = gmHqCache
triAssert [gmHqEstablish ((gmZone (gmZoneIndex "Village")) select 8)]
triAssert [gmHqIndoors]
triAssertEq [(typeOf gmHqProp), "GMHqDesk"]
triAssertEq [(typeOf gmHqCache), "GMHqCacheIndoor"]
triAssertEq [(count (weaponCargo gmHqCache)), 2]
triAssertEq [(count (magazineCargo gmHqCache)), 3]
triSimFrames 10
triAssert [isNull hqOldProp]
triAssert [isNull hqOldCache]
clearWeaponCargo gmHqCache
clearMagazineCargo gmHqCache
triSimFrames 150
triAssert [not (isNull gmHqCache)]
triAssertEq [(count (weaponCargo gmHqCache)), 0]
triAssertEq [(count (magazineCargo gmHqCache)), 0]
triAssertEq [(triSaveGame "hqvisuals"), "OK"]
triEndTest
