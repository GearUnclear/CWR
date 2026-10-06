triSimUntil { not (isNil "GM_AS_READY") }
// A fresh process has neither the native records nor the saved object bank.
triAssertEq [gmAssailantCount "ALL", 0]
triAssert [isNil "asSaveResister"]
triAssertEq [(triLoadGame "assailants"), "OK"]
triSimFrames 3
triAssert [alive asSaveResister]
triAssert [alive asSaveRogue]
triAssert [alive asSaveBystander]
triAssert [not (isNull asSaveDead)]
triAssert [not (alive asSaveDead)]
triAssertEq [typeOf asSaveDead, asSaveDeadType]
triAssertEq [asSaveDeadClassAtDeath, "RESISTER"]
triAssertEq [asSaveDeadClassAtSave, "RESISTER"]
triAssertEq [gmAssailantClass asSaveDead, ""]
triAssertEq [count (units asSaveDeadGroup), 0]
triAssertEq [typeOf asSaveResister, asSaveResType]
triAssertEq [typeOf asSaveRogue, asSaveRogueType]
triAssertEq [gmAssailantClass asSaveResister, "RESISTER"]
triAssertEq [gmAssailantClass asSaveRogue, "ROGUE"]
triAssertEq [gmAssailantClass asSaveBystander, ""]
triAssertEq [gmAssailantCount "ALL", 2]
triAssertEq [gmAssailantCount "RESISTER", 1]
triAssertEq [gmAssailantCount "ROGUE", 1]
triAssertEq [gmAssailantSide, "WEST"]
triAssertEq [format ["%1", side asSaveResister], "WEST"]
triAssertEq [format ["%1", side asSaveRogue], "WEST"]
triAssertEq [format ["%1", side asSaveBystander], "CIV"]
triAssertEq [group asSaveResister, asSaveResGroup]
triAssertEq [group asSaveRogue, asSaveRogueGroup]
triAssertEq [group asSaveBystander, asSaveOldGroup]
triAssert [not (asSaveResGroup == asSaveRogueGroup)]
triAssertEq [count (units asSaveResGroup), 1]
triAssertEq [count (units asSaveRogueGroup), 1]
triAssertEq [count (units asSaveOldGroup), 1]
triAssertEq [format ["%1", weapons asSaveResister], asSaveResWeapons]
triAssertEq [format ["%1", magazines asSaveResister], asSaveResMags]
triAssertEq [format ["%1", weapons asSaveRogue], asSaveRogueWeapons]
triAssertEq [format ["%1", magazines asSaveRogue], asSaveRogueMags]
triAssertEq [GM_AS_REMAINING, 1379]
triAssertEq [GM_PANIC_UNTIL select (GM_PANIC_ZONES find "Village"), asSavePanicUntil]
triAssert [not (gmAssailantRegister [asSaveResister, "RESISTER", player, "Village"])]
triAssertEq [count (magazines asSaveResister), 2]
// Let all restored managers run before enabling combat. No duplicate incident,
// merged group, replaced body or reissued ammunition may appear on resume.
triSimUntil { GM_AS_TICKS > asSaveTicks + 2 }
triSimUntil { not (isNil "GM_pVer") }
triAssertEq [gmAssailantCount "ALL", 2]
triAssertEq [count (units asSaveResGroup), 1]
triAssertEq [count (units asSaveRogueGroup), 1]
triAssertEq [format ["%1", magazines asSaveResister], asSaveResMags]
triAssertEq [format ["%1", magazines asSaveRogue], asSaveRogueMags]
triAssertEq [GM_AS_REMAINING, 1379]
triAssertEq [GM_PANIC_UNTIL select (GM_PANIC_ZONES find "Village"), asSavePanicUntil]
triAssertEq [asSaveShots, 0]
// The extorter reference must relink too: a captive player is perceived CIV,
// and a resister must still retaliate against that specific saved individual.
asSaveResister stop false
asSaveResister enableAI "TARGET"
asSaveResister enableAI "AUTOTARGET"
asSaveResGroup setCombatMode "RED"
triSimUntil { asSaveShots > 0 }
triAssert [captive player]
triAssertEq [gmUndercoverStatus, 0]
triAssertGe [(asSaveResGroup knowsAbout player), 0.1]
triAssert [gmAssailantRemove asSaveResister]
triAssert [gmAssailantRemove asSaveRogue]
triAssertEq [gmAssailantCount "ALL", 0]
triEndTest
