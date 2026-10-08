// Real Classic assets: empty indoor cache, layout transition, inventory and
// old-prop cleanup. Subsequent phases load each layout in a fresh process.
triSimUntil { gmZoneCount > 0 }
player setCaptive true
triAssert [gmHqEstablish ((gmZone (gmZoneIndex "Village")) select 8)]
triAssert [gmHqIndoors]
triAssertEq [(typeOf gmHqCache), "GMHqCacheIndoor"]
triAssertEq [(typeOf gmHqProp), "GMHqDesk"]
triAssertEq [(count (weaponCargo gmHqCache)), 0]
triAssertEq [(count (magazineCargo gmHqCache)), 0]
triAssertGt [(gmHqProp distance gmHqCache), 1.5]
hqOldCache = gmHqCache
hqOldProp = gmHqProp
gmHqCache addWeaponCargo ["AK47", 2]
gmHqCache addMagazineCargo ["AK47", 3]
hqStashes = gmStashCount
triAssert [gmHqEstablish ((gmZone (gmZoneIndex "Outpost")) select 8)]
triAssert [not gmHqIndoors]
triAssertEq [(typeOf gmHqCache), "GMHqCacheOutdoor"]
triAssertEq [(typeOf gmHqProp), "GMHqTent"]
triAssertEq [(count (weaponCargo gmHqCache)), 2]
triAssertEq [(count (magazineCargo gmHqCache)), 3]
triAssertEq [gmStashCount, hqStashes]
triSimFrames 10
triAssert [isNull hqOldCache]
triAssert [isNull hqOldProp]
triAssertGt [(gmHqProp distance gmHqCache), 3]
triAssertLt [(gmHqProp distance gmHqCache), 9]
triAssertEq [(triSaveGame "hqvisuals"), "OK"]
triEndTest
