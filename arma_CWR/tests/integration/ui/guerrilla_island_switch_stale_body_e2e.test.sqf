// ============================================================================
//  Guerrilla Mode - a CHARACTER pick is durable across an island switch, and
//  its addon closure is activated at launch (issue #45).
//
//  THE POINT, in two halves:
//
//  1. THE ROSTER IS PACKAGE-WIDE BY DESIGN (issue #43's uncapped mod-aware
//     roster). _bodies is built once from Pars >> CfgVehicles, which is
//     process-global and identical on every island, so an island change must
//     NOT discard a body the player deliberately picked. Occupier/resistance/
//     outfit DO re-seed on that change, because those three come from the
//     island's own CfgGuerrillaFactions (its template description.ext) and are
//     genuinely invalidated by it. RevalidateBodySelection re-renders the
//     CHARACTER label and the mannequin on every island change so the button
//     can never promise a body the launch will not deliver.
//
//  2. THE PICKED BODY'S ADDONS ARE ACTIVATED AT THE SUBSTITUTION SEAM.
//     Guerrilla.Abel is a vanilla template: its mission.sqm authors no
//     addOns[], and it deliberately never will - listing @LoBo owners there
//     would make a stock island hard-require the mod (CheckPatch fills
//     missingAddOns, ArcadeTemplate::Serialize returns LSNoAddOn). So
//     ApplyPlayerOutfitSelection (Game/Guerrilla/OutfitSelect.cpp) instead
//     ADDITIVELY activates the picked body's owner plus the owners of its
//     weapons[]/magazines[], from inside World::InitVehicles - after
//     ActivateAddons(CurrentTemplate.addOns) has reset the list, and before any
//     unit is created. Without it, World::CheckAddon denies 'lobois',
//     'loboweapons' and 'loboweapnad', and each denial puts an
//     IDS_MSG_ADDON_MISSING warning on the player's screen. The triAddonActive
//     assertions below are the deterministic in-mission read of that list (the
//     WarningMessage box itself is not reachable from SQF); they go RED against
//     the pre-fix engine.
//
//  Flow: main menu -> GUERRILLA (120) -> Sinai -> CHARACTER (155) -> IDD 77 ->
//  select "lobogolaniw" (LoBoGolaniWB, an @LoBo IDF WEST body) -> CONFIRM ->
//  back on IDD 76 switch the island list (101) to Abel -> OK -> in-mission.
//
//  PRECONDITIONS (see the .toml): templates installed via
//  install-missions.ps1 -IncludeWorld Sinai,Lebanon80; the @lobofixup patched
//  pbos generated once.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- GUERRILLA entry -> new-game display -> Sinai ----------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "sinai"]), true]
triAssertEq [(triControlText 155), "CHARACTER: (match outfit)"]

// -- pick the @LoBo Golani body on the character screen and CONFIRM ----------
triClick 155
triAssertEq [(triDisplay), 77]
triAssertEq [(triSelectListByData [160, "lobogolaniw"]), true]
triAssertEq [(triLBText [160, (triLBCurSel 160)]), "Sayeret Golani Operator"]
triClick 1
triAssertEq [(triDisplay), 76]
triAssertEq [(triControlText 155), "CHARACTER: Sayeret Golani Operator"]

// -- switch the island to Abel. The CHARACTER pick is KEPT (package-wide
//    roster); only the island-scoped selections re-seed. One combined readout
//    so a FAIL message reports every label at once. -------------------------
triAssertEq [(triSelectListByData [101, "abel"]), true]
triAssertEq [(format ["%1 || %2 || %3", (triControlText 155), (triControlText 150), (triControlText 151)]), "CHARACTER: Sayeret Golani Operator || OCCUPIER: EAST || RESISTANCE: GUER"]
triScreenshot "abel_selected_golani_kept"

// -- OK launches the installed Guerrilla.Abel template -----------------------
triClick 1
triSimUntil { alive player }

// -- what actually spawned (combined readout again) --------------------------
triAssertEq [(format ["%1 || %2 || %3", (typeOf player), (side player), gmSelPlayerClass]), "LoBoGolaniWB || GUER || LoBoGolaniWB"]
triAssertEq [(format ["%1 || %2 || %3", gmSelIsland, gmSelOccupier, gmSelResistance]), "Abel || EAST || GUER"]

// -- the mod loadout the body is issued -------------------------------------
triAssertEq [(primaryWeapon player), "LoBo_M4A1_LD_G_Falcon"]
triAssertIncludes [(format ["%1", (magazines player)]), "JAM_W556_30mag"]

// -- THE FIX: every owner in the picked body's addon closure is active, so
//    World::CheckAddon grants instead of denying and no IDS_MSG_ADDON_MISSING
//    warning reaches the player. 'lobois' owns the body, 'loboweapons' and
//    'loboweapnad' own its weapons[]/magazines[] - the three owners the
//    pre-fix run logged as "Access denied:". One combined readout so a FAIL
//    names all three at once. ------------------------------------------------
triAssertEq [(format ["%1 || %2 || %3", (triAddonActive "lobois"), (triAddonActive "loboweapons"), (triAddonActive "loboweapnad")]), "true || true || true"]

// mission display, i.e. no modal addon-warning box latched over the game
triAssertEq [(triDisplay), 46]
triScreenshot "abel_golani_in_mission"

triEndTest
