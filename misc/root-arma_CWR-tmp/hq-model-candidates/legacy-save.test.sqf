triSimUntil { gmZoneCount > 0 }
triAssert [gmHqEstablish ((gmZone (gmZoneIndex "Village")) select 8)]
triAssert [gmHqIndoors]
gmHqCache addWeaponCargo ["AK47", 2]
gmHqCache addMagazineCargo ["AK47", 3]
deleteVehicle gmHqProp
triSimFrames 1
triAssertEq [(triSaveGame "legacyhq"), "OK"]
triEndTest
