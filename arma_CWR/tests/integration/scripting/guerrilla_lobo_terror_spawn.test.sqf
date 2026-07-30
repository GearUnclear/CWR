// ============================================================================
//  Guerrilla Mode - LoBo Terror-class spawn smoke test (issue #25 M2.4).
//
//  The Sinai/Lebanon80 civilian-outfit family is the LoBoTer Terror roster
//  (*E side twins). Those classes carry Init/fired EventHandlers exec'ing
//  \LoBoTer\scripts\* plus ECP handler references - untested under
//  createUnit until now. This boots the real Guerrilla.Sinai template
//  (direct launch: no gmSelOutfit published, so the campaign itself stays
//  warrior-bodied) and spawns each descriptor-authored Terror class into a
//  resistance-side group next to the player. Script errors in the classes'
//  Init EHs hard-abort via --autotest; the asserts pin that the bodies
//  exist, are alive and keep their class after a settle window.
// ============================================================================

triSimUntil { GM_LIB_READY }

// direct launch: the outfit flag folded to false (nil gmSelOutfit)
triSimUntil { format ["%1", GM_OUTFIT_CIV] == "false" }

// the loaded package carries the whole *E family (fails loud on a swap)
triSimUntil { gmClassExists "LoBo_Terror_01E" }
triSimUntil { gmClassExists "LoBo_Terror_02E" }
triSimUntil { gmClassExists "LoBo_Terror_MG2E" }

// the registry resolved the civTier[] rung to the authored Terror class
triAssertEq [(gmFactionCivTier [gmResistanceSide, 1]), "LoBo_Terror_01E"]
// and the *Civ scalar keys survived the plan-15 pass un-substituted
triAssertEq [(gmFactionValue [gmResistanceSide, "playerClassCiv"]), "LoBo_Terror_01E"]
triAssertEq [(gmFactionValue [gmResistanceSide, "recruitFighterCiv"]), "LoBo_Terror_02E"]
triAssertEq [(gmFactionValue [gmResistanceSide, "recruitSpecialistCiv"]), "LoBo_Terror_MG2E"]
triAssertEq [(gmFactionValue [gmResistanceSide, "holdClassCiv"]), "LoBo_Terror_02E"]

// -- spawn the family into a fresh resistance-side group ---------------------
// (side aP = the campaign resistance side; instance side comes from the
// owning center, never the Terror class's config side)
ltGrp = createGroup (side player)
ltPos = getPos player
"LoBo_Terror_01E" createUnit [[(ltPos select 0) + 8, (ltPos select 1) + 4, 0], ltGrp, "ltU1 = this", 0.4, "PRIVATE"]
"LoBo_Terror_02E" createUnit [[(ltPos select 0) + 10, (ltPos select 1) + 4, 0], ltGrp, "ltU2 = this", 0.4, "PRIVATE"]
"LoBo_Terror_MG2E" createUnit [[(ltPos select 0) + 12, (ltPos select 1) + 4, 0], ltGrp, "ltU3 = this", 0.4, "PRIVATE"]
triSimUntil { (count units ltGrp) >= 3 }

// -- settle window: the classes' Init EHs (\LoBoTer\scripts\*, ECP refs)
//    run at spawn; a script error aborts the run here under --autotest ------
ltT0 = time
triSimUntil { time > ltT0 + 12 }

triSimUntil { alive ltU1 }
triSimUntil { alive ltU2 }
triSimUntil { alive ltU3 }
triAssertEq [(typeOf ltU1), "LoBo_Terror_01E"]
triAssertEq [(typeOf ltU2), "LoBo_Terror_02E"]
triAssertEq [(typeOf ltU3), "LoBo_Terror_MG2E"]
// the group welded them to the resistance side regardless of config side
triAssertEq [(format ["%1", side ltU1]), (format ["%1", side player])]

triEndTest
