// ============================================================================
//  Phase 2 of the MOD-BODY save/reload WITHOUT THE MOD (issue #48, #56 task 5).
//    Same Classic package, same user dir, but this process mounts NO mod at
//    all (see the .toml: no --mod line). Phase 1 wrote UserDir/Saved/Tmp/
//    globo.fps with the player as LoBoGolaniWB carrying a LoBo M4A1 and JAM
//    magazines, and with lobois/loboweapons/loboweapnad in the save's
//    "addons" list. None of those classes or addons exist here.
//
//    The contract under test is the plan-15 one: unknown content degrades
//    with a log line, it never takes the campaign down. So the load must
//    answer, the process must be alive afterwards, and the player must exist
//    as a body on the GUER side. The substitute class is read back explicitly
//    so a change to what the engine picks shows up as a diff, and the
//    addon-active reads pin that the save's grant for absent addons stays
//    inert rather than faking "true".
// ============================================================================
triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- fresh Abel with the character screen untouched: the clean-slate baseline --
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "abel"]), true]
triAssertEq [(triControlText 155), "CHARACTER: (match outfit)"]
triClick 1
triSimUntil { alive player }
triAssertEq [(format ["%1", isNil "gmSelPlayerClass"]), "true"]
triAssertEq [(typeOf player), "SoldierGB"]
// no mod mounted: the three @LoBo owners are unknown, so not active
triAssertEq [(format ["%1 || %2 || %3", (triAddonActive "lobois"), (triAddonActive "loboweapons"), (triAddonActive "loboweapnad")]), "false || false || false"]

// -- load phase 1's save into a process that has never heard of @LoBo --------
triAssertEq [(triLoadGame "globo"), "OK"]
triSimFrames 3

// -- the campaign survived: a player exists, on the resistance side -----------
triAssertEq [(format ["%1", alive player]), "true"]
triAssertEq [(format ["%1", (side player)]), "GUER"]

// -- the body the engine rebuilt in place of the missing LoBoGolaniWB ---------
//    Pinned by value so a change in the substitution ladder is a visible diff.
//    The class must be one the Classic package ships (a plain existence read
//    through the harness), never the serialized mod name.
triAssertEq [(format ["%1", ((typeOf player) == "LoBoGolaniWB")]), "false"]
triAssertEq [(format ["%1", ((typeOf player) != "")]), "true"]

// -- the grant for absent addons stays inert --------------------------------
triAssertEq [(format ["%1 || %2 || %3", (triAddonActive "lobois"), (triAddonActive "loboweapons"), (triAddonActive "loboweapnad")]), "false || false || false"]

// -- the pick itself still rides the GGameState bank (it is a string, not a class)
triAssertEq [gmSelPlayerClass, "LoBoGolaniWB"]

// -- and the restored world keeps running -------------------------------------
glR0 = triFrameCount
triSimUntil { triFrameCount > glR0 + 300 }
triAssertEq [(format ["%1 || %2", (alive player), (side player)]), "true || GUER"]
triEndTest
