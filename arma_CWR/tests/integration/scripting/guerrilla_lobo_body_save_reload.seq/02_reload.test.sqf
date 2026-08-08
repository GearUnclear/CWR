// ============================================================================
//  Phase 2 of the MOD-BODY save/reload round-trip (issue #48 gap 2).
//    Fresh menu launch of the SAME vanilla Guerrilla.Abel with the character
//    screen UNTOUCHED. The clean-slate baseline is asserted first and it is
//    what gives the diff its meaning:
//
//      * the authored SoldierGB and a nil gmSelPlayerClass - so nothing here
//        can false-pass on leftover state in the shared user dir;
//      * and the three @LoBo owners report INACTIVE even though @LoBo is
//        mounted and its fixup preloads name them. Mounting a mod is not
//        activating its addons, so the post-load "true" below is genuinely
//        carried by the savegame and not by the mod mount.
//
//    Then triLoadGame rehydrates phase 1's world. Two independent things have
//    to have survived: the unit, which Entity::CreateObject rebuilds from the
//    serialized class string rather than from the template (InitVehicles - the
//    substitution site - never runs on the load path), and World::_activeAddons,
//    which World::Serialize writes as "addons" and re-activates on load. If
//    only the first survived, the body would come back but its weapon and
//    magazine classes would be denied.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// -- same menu path, character screen untouched -------------------------------
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "abel"]), true]
triAssertEq [(triControlText 155), "CHARACTER: (match outfit)"]
triClick 1
triSimUntil { alive player }

// -- baseline: fresh-boot defaults, NOT the sentinels --------------------------
triAssertEq [(format ["%1", isNil "gmSelPlayerClass"]), "true"]
triAssertEq [(typeOf player), "SoldierGB"]
// These three owners now report ACTIVE on a fresh boot, and that is a fix, not a
// leak: @lobofixup's bin/config.cpp names them in CfgAddons/PreloadAddons, and
// that roster finally reaches World::ActivateAddons. It used to be swallowed
// twice over - ConfigParsers kept only ONE mod's deferred bin/config, and the
// stock config locks CfgAddons at PAReadOnly so ParamClass::Update refused it
// silently (AccessDenied reports through RptF, compiled out). So the old
// "false || false || false" was recording two engine bugs, not a property of
// mod mounting. See AddonSystem::MergeConfigInto.
//
// The consequence for THIS test: this line is no longer the control for DIFF 2
// below, because a preloaded owner would report active either way. The control
// that still bites is the one right above - gmSelPlayerClass nil and the
// authored SoldierGB - plus DIFF 3, which needs the actual mod loadout to
// rebuild and would fail per-entity if the owners had come back denied.
triAssertEq [(format ["%1 || %2 || %3", (triAddonActive "lobois"), (triAddonActive "loboweapons"), (triAddonActive "loboweapnad")]), "true || true || true"]

// -- restore phase 1's binary save --------------------------------------------
triAssertEq [(triLoadGame "globo"), "OK"]
triSimFrames 3

// -- DIFF 1: the mod body persisted, rebuilt from the serialized class ---------
triAssertEq [(format ["%1 || %2", (typeOf player), (side player)]), "LoBoGolaniWB || GUER"]

// -- DIFF 2: the runtime addon grant rode the save -----------------------------
//    This is the half issue #45 recorded as unverified.
triAssertEq [(format ["%1 || %2 || %3", (triAddonActive "lobois"), (triAddonActive "loboweapons"), (triAddonActive "loboweapnad")]), "true || true || true"]

// -- DIFF 3: and it is not merely listed - the mod loadout rebuilt too, which
//    is what would fail per-entity if the owners had come back denied ---------
triAssertEq [(primaryWeapon player), "LoBo_M4A1_LD_G_Falcon"]
triAssertIncludes [(format ["%1", (magazines player)]), "JAM_W556_30mag"]

// -- DIFF 4: the pick itself rides the GGameState bank in the save -------------
triAssertEq [gmSelPlayerClass, "LoBoGolaniWB"]

// -- and it stays put once the restored world has run on for a while -----------
glR0 = triFrameCount
triSimUntil { triFrameCount > glR0 + 300 }
triAssertEq [(format ["%1 || %2", (typeOf player), (side player)]), "LoBoGolaniWB || GUER"]

triEndTest
