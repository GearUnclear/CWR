// Issue #79 B4, phase 1: GUERRILLA -> Abel -> new campaign, stamp a state,
// save it with the campaign's own "Save Game" action, and prove the pause
// menu's Load reads what that action wrote (the save-slot contract,
// UI/MissionSaves.hpp: the action writes save.fps, the user slot).

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]

// a fresh profile: no campaign save, so no CONTINUE and no replace prompt
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triAssertEq [(triGetControlVisible 156), "0"]
triClick 1
triSimUntil { alive player }
triSimUntil { GM_LIB_READY }

// stamp what the later phases must get back: resources, a zone owner and
//    the player's position (kept in a script global, which rides the save)
gsOut = gmZoneIndex "Outpost"
triAssertGe [gsOut, 0]
gmZoneSet [gsOut, "owner", gmResistanceSide]
gmResources = 4242
player setPos [((getPos player) select 0) + 12, ((getPos player) select 1) + 12, 0]
triSimFrames 5
UD_B4_POS = getPos player

// save through the action menu entry campaign.sqs registered
triAssertIncludes [(triActionMenuText), "Save Game"]
player action ["USER", player, GM_pSaveAct]
triSimUntil { ((gmJournalEntry (gmJournalCount - 1)) select 1) == "Record kept." }
triSimFrames 5

// the pause menu offers Save AND Load for a Guerrilla campaign, and Load
//    restores the action's save, not some other slot
gmResources = 1
triSendKey 41
triAssertEq [(triDisplay), 49]
triAssertEq [(triGetControlVisible 103), "1"]
triAssertEq [(triGetControlVisible 102), "1"]
triScreenshot "01_pause_save_and_load"
triClick 102
triSimUntil { alive player }
triSimFrames 5
triAssertEq [gmResources, 4242]
triAssertEq [((gmZone gsOut) select 2), gmResistanceSide]

triEndTest
