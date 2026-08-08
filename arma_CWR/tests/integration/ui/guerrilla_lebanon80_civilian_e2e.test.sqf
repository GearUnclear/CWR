// ============================================================================
//  Guerrilla Mode - the CIVILIAN outfit on Lebanon80, through the real menu,
//  and the island-scoped lifetime of the outfit token (issue #48 gap 1).
//
//  WHY THIS EXISTS. Every earlier proof of the menu-to-mission body seam ran
//  on Guerrilla.Abel: one island, one vanilla descriptor, one warrior/civilian
//  pair. Lebanon80 had never been launched from the menu at all, and the
//  civilian-drop path in RefreshOutfitChoices had never been reached, so three
//  things were unmeasured:
//
//    1. THE MODDED CIVILIAN SUBSTITUTION. Lebanon80's resistance descriptor
//       (Hizballah, side EAST) authors playerClassCiv = "LoBo_Terror_01E",
//       whose owner LoBoTerror IS in the template's addOns[] - unlike a
//       CHARACTER body pick, which is package-wide and needs issue #45's
//       runtime grant. So this is the other half of the addon story: the
//       descriptor-sourced civilian body must resolve with no grant at all.
//
//    2. THE DROP. An island with no Guerrilla template at all (Cain, Eden,
//       Everon, Intro on this package) leaves _islandFactions null, so
//       GuerrillaOutfitChoices returns an empty pair, _outfitSel is forced to
//       -1 and NOTHING is published - the authored mission.sqm class decides.
//       The loss is one-way by design (issue #46 seam 6): `keep` is re-derived
//       from the list that just emptied, so a trip BACK to an island that does
//       offer CIVILIAN reopens on WARRIOR. That contract is pinned below so a
//       future change to it is a deliberate one.
//
//    3. THE FACTION NAME-KEEP. Hopping Abel -> Lebanon80 used to open on
//       OCCUPIER: Hizballah / RESISTANCE: Hizballah, because the keep matched
//       Abel's occupier (a class literally NAMED "EAST") through the side rung
//       of GuerrillaIndexOfSelection against Hizballah (side = "EAST"), which
//       is Lebanon80's defaultResistance. The authored defaultOccupier = "IDF"
//       was discarded silently and the campaign opened fighting itself. The
//       keep is GuerrillaIndexOfName now - identity, no side rung - so a name
//       the new island does not carry falls through to its own default* keys.
//       Section 5 goes RED against that.
//
//  LONGER HORIZON (issue #48 gap 3). Every previous in-mission read of the
//  substituted body happened within ~10 simulated frames of spawn, which
//  proves the substitution beats ::CreateCenter but leaves everything
//  downstream unmeasured. Section 8 re-reads the body only after the zone
//  registry has seeded, the manager scripts have folded the outfit token, a
//  native garrison has actually spawned live occupier units near the player,
//  and several hundred further frames have run.
//
//  PRECONDITIONS (see the .toml): install-missions.ps1 -IncludeWorld
//  Sinai,Lebanon80; the one-time @lobofixup patched pbos.
// ============================================================================

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]

// == 1. Lebanon80 opens on its OWN authored defaults =========================
//    defaultOccupier = "IDF", defaultResistance = "Hizballah"
//    (CfgGuerrillaZones), and Hizballah authors playerClassCiv so the pair is
//    offered. Whatever the cyclers show is what OK publishes, and a published
//    selection outranks the template's default* keys - so this line is the
//    "visiting the menu does not rewrite the campaign" contract for the third
//    island pack.
triAssertEq [(triSelectListByData [101, "lebanon"]), true]
triAssertEq [(format ["%1 || %2 || %3 || %4", (triControlText 150), (triControlText 151), (triControlText 153), (triControlText 155)]), "OCCUPIER: IDF || RESISTANCE: Hizballah || OUTFIT: WARRIOR || CHARACTER: (match outfit)"]

// the mannequin resolves LoBo_HizballahRifle4's model out of @LoBo
triAssertEq [(triGetControlVisible 154), "1"]
triClick 153
triAssertEq [(triControlText 153), "OUTFIT: CIVILIAN"]
triAssertEq [(triGetControlVisible 154), "1"]

// == 2. Abel also offers the pair, so the token survives BY NAME =============
//    The faction cyclers re-seed (neither IDF nor Hizballah exists on Abel),
//    but the outfit token is kept - the positive half of the seam-6 contract.
triAssertEq [(triSelectListByData [101, "abel"]), true]
triAssertEq [(format ["%1 || %2 || %3", (triControlText 150), (triControlText 151), (triControlText 153)]), "OCCUPIER: EAST || RESISTANCE: GUER || OUTFIT: CIVILIAN"]

// == 3. THE DROP: an island with no Guerrilla template =======================
//    No descriptor at all -> no pair -> _outfitSel = -1 -> the row renders the
//    publish-nothing state and a WARN goes to the log.
triAssertEq [(triSelectListByData [101, "cain"]), true]
triAssertEq [(triControlText 153), "OUTFIT: (mission default)"]

// == 4. THE LOSS IS ONE-WAY, deliberately ====================================
//    Back on an island that DOES offer the pair, the row reopens on WARRIOR:
//    `keep` was re-derived from the emptied list, so the token is gone from
//    the object. Pinned so that making it durable is a deliberate change.
triAssertEq [(triSelectListByData [101, "abel"]), true]
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]

// == 5. THE NAME-KEEP MUST NOT ALIAS ONTO A SIDE =============================
//    Abel's occupier is the class literally named EAST. Lebanon80 carries no
//    faction of that NAME, so the keep must miss and the template's own
//    default* pair must win. The pre-fix engine matched EAST through the side
//    rung onto Hizballah and opened on Hizballah vs Hizballah.
triAssertEq [(triSelectListByData [101, "lebanon"]), true]
triAssertEq [(format ["%1 || %2", (triControlText 150), (triControlText 151)]), "OCCUPIER: IDF || RESISTANCE: Hizballah"]
triAssertEq [(triControlText 153), "OUTFIT: WARRIOR"]

// == 6. re-pick CIVILIAN on Lebanon80 and launch =============================
triClick 153
triAssertEq [(triControlText 153), "OUTFIT: CIVILIAN"]
triScreenshot "lebanon80_civilian_selected"

triClick 1
triSimUntil { alive player }

// == 7. what actually spawned ================================================
//    The published pair, then the body. LoBo_Terror_01E is the Hizballah
//    block's playerClassCiv; the authored mission.sqm class is
//    LoBo_HizballahRifle4, so a failure to substitute reads as that.
triAssertEq [(format ["%1 || %2 || %3 || %4", gmSelIsland, gmSelOccupier, gmSelResistance, gmSelOutfit]), "Lebanon80 || IDF || Hizballah || CIVILIAN"]
triAssertEq [(format ["%1", isNil "gmSelPlayerClass"]), "true"]
triAssertEq [(format ["%1 || %2", (typeOf player), (side player)]), "LoBo_Terror_01E || EAST"]

//    The civilian body is descriptor-sourced, and its owner LoBoTerror is in
//    the template's own addOns[] - so unlike a CHARACTER pick it needs no
//    runtime addon grant. Assert the owner really is active, i.e. no
//    "Access denied" path was taken to get here.
triAssertEq [(triAddonActive "loboterror"), true]
//    mission display, i.e. no modal addon-warning box latched over the game
triAssertEq [(triDisplay), 46]

// == 8. LONGER HORIZON (issue #48 gap 3) =====================================
//    Nothing above ran more than a handful of frames past spawn. Advance the
//    world through everything that could plausibly touch the player unit and
//    read the body again.

//    (a) init.sqs + the shared core to completion
triSimUntil { GM_LIB_READY }
triSimUntil { format ["%1", GM_OUTFIT_CIV] == "true" }
triSimUntil { format ["%1", GM_RECRUIT_FIGHTER] == "LoBo_Terror_02E" }
triSimUntil { format ["%1", GM_COMP_CLASS] == "LoBo_Terror_01E" }

//    (b) the native registry seeded and resolved both sides
triSimUntil { gmZoneCount >= 6 }
triAssertEq [(format ["%1 || %2", gmOccupierSide, gmResistanceSide]), "WEST || EAST"]

//    (c) AI side resolution with real units: the garrison cache spawns live
//        IDF units at the outpost 500 m from the player's Camp
gcIdx = gmZoneIndex "Litani Checkpoint"
triSimUntil { gmGarrisonSpawned gcIdx }
triSimUntil { (gmGarrisonLive gcIdx) > 0 }

//    (d) and then a long quiet stretch on top of all of it
gcF0 = triFrameCount
triSimUntil { triFrameCount > gcF0 + 600 }

//    (e) the body is unchanged, and so is the side weld and the folded token
triAssertEq [(format ["%1 || %2 || %3", (typeOf player), (side player), gmSelOutfit]), "LoBo_Terror_01E || EAST || CIVILIAN"]
triAssertEq [(format ["%1", alive player]), "true"]
triAssertEq [(format ["%1", GM_OUTFIT_CIV]), "true"]
triScreenshot "lebanon80_civilian_in_mission"

triEndTest
