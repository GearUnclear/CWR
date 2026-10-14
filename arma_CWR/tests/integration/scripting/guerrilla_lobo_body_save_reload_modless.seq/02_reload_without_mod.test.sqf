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

// -- the script layer follows the rebuilt body ---------------------------------
//    The saved aP / gmPlayer handles pointed at the dropped LoBoGolaniWB body.
//    campaign.sqs's campaignLoaded handler rebinds them to the live player and
//    re-arms the Save action on the NEW body.
triSimUntil { not (isNull aP) }
triAssertEq [(format ["%1 || %2", (aP == player), (gmPlayer == player)]), "true || true"]
triSimUntil { GM_pSaveAct >= 0 }
glS0 = triFrameCount
triSimUntil { triFrameCount > glS0 + 10 }
triAssertIncludes [(triActionMenuText), "Save Game"]

// -- and the HQ menus mount on it: stand at the HQ, the recruit menu appears ---
//    (the default Abel start is a Camp HQ; the save recorded the recruit menu
//    as mounted on the old body, so this only goes green if the load handler
//    cleared that stale mount)
triSimUntil { gmHqEstablished }
player setPos [(gmHqPos select 0) + 3, (gmHqPos select 1) + 3, 0]
triSimUntil { gmRecruitActive and (gmActRecruit >= 0) }
glH0 = triFrameCount
triSimUntil { triFrameCount > glH0 + 10 }
triAssertIncludes [(triActionMenuText), "Recruit Fighter (1 HR)"]
triAssertIncludes [(triActionMenuText), "Save Game"]

// -- and the restored world keeps running -------------------------------------
glR0 = triFrameCount
triSimUntil { triFrameCount > glR0 + 300 }
triAssertEq [(format ["%1 || %2", (alive player), (side player)]), "true || GUER"]
triEndTest
