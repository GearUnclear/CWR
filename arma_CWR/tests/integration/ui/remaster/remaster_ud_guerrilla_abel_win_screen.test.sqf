// Guerrilla campaign win, end to end on the official Remaster (issue #79 B3):
// the win latch asks for END1, the mission ends into the stock debriefing
// (IDD_DEBRIEFING = 50) carrying the UD win text in its right page
// (IDC_DEBRIEFING_RIGHT = 102), and Continue (IDC_CANCEL = 2) runs the empty
// OutroWin and lands on the main menu (display 0).

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

// a short end window so the run stays quick; the latch is asserted inside it
gmCampaignEndDelay 6
triSimUntil { ((gmCampaignEnd) select 5) > 0 }
cwI = 0
while {cwI < gmZoneCount} do {cwZ = gmZone cwI; if (((cwZ select 1) != "CAMP") and ((cwZ select 1) != "CITY") and ((cwZ select 2) == gmOccupierSide)) then {gmZoneSet [cwI, "owner", gmResistanceSide]}; cwI = cwI + 1}
triSimUntil { gmCampaignWon }
triAssertEq [((gmCampaignEnd) select 2), false]

// -- the end: the debriefing with the UD win text ------------------------------
triWaitUntil { triDisplay == 50 }
triAssertIncludes [(triControlText 102), "Malden is free."]
triAssertIncludes [(triControlText 102), "Every occupier base on Malden is in resistance hands"]
// let the notebook finish opening before the capture
triWaitFrames 120
triScreenshot "abel_campaign_won_debriefing"

// -- Continue: the empty OutroWin, then the main menu ---------------------------
triInvokeButton 2
triWaitUntil { triDisplay == 0 }
triScreenshot "abel_campaign_won_main_menu"
triEndTest
