// Guerrilla campaign loss, end to end on the official Remaster
// (OCCUPIER-FORCES.md 9.2): no HQ, no base and no town latches the loss, the
// end clock asks for EMLoser (never END2), the mission ends into the stock
// debriefing (IDD_DEBRIEFING = 50) carrying the UD lose text in its right
// page (IDC_DEBRIEFING_RIGHT = 102), and Continue (IDC_CANCEL = 2) runs
// OutroLoose and lands on the main menu (display 0).

triSetLanguage "English"
triSimUntil { triGameMode == 2 }
triAssertEq [(triDisplay), 0]
triAssertEq [(triClick 120), true]
triAssertEq [(triDisplay), 76]
triAssertEq [(triSelectListByData [101, "Abel"]), true]
triClick 1
triSimUntil { alive player }
triAssertEq [gmSelIsland, "Abel"]
triSimUntil { GM_LIB_READY }
player allowDammage false

// the "(camp)" default: the campaign's HQ stands at the Camp
triSimUntil { gmHqEstablished }
triAssertEq [gmHqZone, "Camp"]

// a short end window so the run stays quick; the latch is asserted inside it
gmCampaignEndDelay 6
triSimUntil { ((gmCampaignEnd) select 5) > 0 }
triAssertEq [(format ["%1", (gmCampaignEnd) select 8]), "false"]
// nothing held: every resistance zone (the Camp) goes to the occupier
cwI = 0
while {cwI < gmZoneCount} do {cwZ = gmZone cwI; if ((cwZ select 2) == gmResistanceSide) then {gmZoneSet [cwI, "owner", gmOccupierSide]}; cwI = cwI + 1}
// the HQ still stands: not lost yet
cwT = time
triSimUntil { time > cwT + 3 }
triAssertEq [(format ["%1", gmCampaignLost]), "false"]
// the HQ falls with nothing held: lost
gmHqForceCapture 100
triAssertEq [(format ["%1", gmHqEstablished]), "false"]
triSimUntil { gmCampaignLost }
triAssertEq [(format ["%1", gmCampaignWon]), "false"]
triAssertEq [(format ["%1", (gmCampaignEnd) select 8]), "true"]
triAssertEq [(format ["%1", (gmCampaignEnd) select 2]), "false"]

// -- the end: the debriefing with the UD lose text -----------------------------
triWaitUntil { triDisplay == 50 }
triAssertIncludes [(triControlText 102), "Malden is lost."]
triAssertIncludes [(triControlText 102), "The headquarters is gone and the resistance holds no base and no town on Malden"]
// let the notebook finish opening before the capture
triWaitFrames 120
triScreenshot "abel_campaign_lost_debriefing"

// -- Continue: OutroLoose, then the main menu ----------------------------------
triInvokeButton 2
triWaitUntil { triDisplay == 0 }
triScreenshot "abel_campaign_lost_main_menu"
triEndTest
