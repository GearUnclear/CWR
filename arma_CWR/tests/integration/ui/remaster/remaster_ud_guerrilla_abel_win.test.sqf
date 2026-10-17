// Guerrilla campaign win rule on the official Remaster (issue #79 B3).
// The rule: the campaign is won when every zone that is neither CAMP nor
// CITY and was the occupier's at campaign start is resistance-owned. The set
// is fixed on the first active tick; the win latches once, is saved, writes
// ONE diary line and asks for the END1 debriefing campaignEndDelay seconds
// later. This lane holds the end (gmCampaignEndDelay 3600) and proves:
//   * the roster is every authored enemy military zone of the template
//     (counted here from the zone table, never hardcoded: Abel's occupation
//     network grows);
//   * all but one held does not win;
//   * the last one latches: gmCampaignWon, the pending end, the native
//     victory line naming the last base, the campaign.sqs notice;
//   * losing a base afterwards keeps the win and repeats nothing.

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

// hold the end: an END1 under --autotest exits the game
gmCampaignEndDelay 3600

// -- the roster: every authored enemy military zone ----------------------------
triSimUntil { ((gmCampaignEnd) select 5) > 0 }
cwIdx = []
cwI = 0
while {cwI < gmZoneCount} do {cwZ = gmZone cwI; if (((cwZ select 1) != "CAMP") and ((cwZ select 1) != "CITY") and ((cwZ select 2) == gmOccupierSide)) then {cwIdx = cwIdx + [cwI]}; cwI = cwI + 1}
cwN = count cwIdx
triAssertGe [cwN, 1]
triAssertEq [((gmCampaignEnd) select 5), cwN]
triAssertEq [((gmCampaignEnd) select 4), 0]
triAssertEq [gmCampaignWon, false]

// -- all but the last held: no win --------------------------------------------
cwK = 0
while {cwK < (cwN - 1)} do {gmZoneSet [cwIdx select cwK, "owner", gmResistanceSide]; cwK = cwK + 1}
triSimUntil { ((gmCampaignEnd) select 4) == (cwN - 1) }
cwT0 = time
triSimUntil { time > cwT0 + 3 }
triAssertEq [gmCampaignWon, false]
triAssertEq [((gmCampaignEnd) select 1), false]

// -- the last one: the latch ------------------------------------------------
cwLast = (gmZone (cwIdx select (cwN - 1))) select 0
gmZoneSet [cwIdx select (cwN - 1), "owner", gmResistanceSide]
triSimUntil { gmCampaignWon }
cwState = gmCampaignEnd
triAssertEq [(cwState select 0), true]
triAssertEq [(cwState select 1), true]
triAssertEq [(cwState select 2), false]
triAssertGe [(cwState select 3), 3000]
triAssertEq [(cwState select 4), cwN]
triAssertEq [(cwState select 5), cwN]
triAssertGe [(cwState select 6), 1]
triAssertEq [(cwState select 7), cwLast]

// -- the native victory line, exactly once -------------------------------------
cwLine = format ["%1 is ours. It was the last base they held on Malden.", cwLast]
cwHits = 0
cwI = 0
while {cwI < gmJournalCount} do {if (((gmJournalEntry cwI) select 1) == cwLine) then {cwHits = cwHits + 1}; cwI = cwI + 1}
triAssertEq [cwHits, 1]

// -- campaign.sqs tells the player once (its loop polls every 2 s) ----------
triSimUntil { GM_pWonSeen }

// -- a base lost afterwards keeps the win and repeats nothing ----------------
gmZoneSet [cwIdx select 0, "owner", gmOccupierSide]
cwT0 = time
triSimUntil { time > cwT0 + 4 }
triAssertEq [gmCampaignWon, true]
triAssertEq [((gmCampaignEnd) select 4), cwN - 1]
triAssertEq [((gmCampaignEnd) select 1), true]
cwHits = 0
cwI = 0
while {cwI < gmJournalCount} do {if (((gmJournalEntry cwI) select 1) == cwLine) then {cwHits = cwHits + 1}; cwI = cwI + 1}
triAssertEq [cwHits, 1]
triScreenshot "abel_campaign_won_pending"

triEndTest
