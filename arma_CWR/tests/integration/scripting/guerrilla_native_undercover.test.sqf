// ============================================================================
//  Guerrilla Mode NATIVE parity - undercover break detection (issue #3 item 5).
//    The retired alert.sqs cover-break poll is engine code now
//    (AlertMachine::EvaluateAlert). This proves, on full CWA data (Abel):
//      1. the mission establishes cover at boot (undercover.sqs:
//         gmUndercover = true + setCaptive true);
//      2. gmBreakUndercover "fired" - the EXACT call the mission's fired-EH
//         makes (undercover.sqs:33) - is latched and consumed by the next
//         native alert tick;
//      3. the native undercoverBroken event fires and the scripted reaction
//         runs: gmUndercover drops to false and the player is un-captived;
//      4. the engine spiked Heat on the zone nearest the player (the Camp,
//         where the player stands) by alertHeatBreak (25).
//
//  The break enters through the native command the fired-EH uses, then flows
//  alert tick -> event -> undercover.sqs reaction - no manager globals are
//  poked. (A literal `player fire "AK47CZ"` would be the full chain, but
//  force-firing the real player crashes the engine headless - 0xC0000005 in
//  Man::ProcessMoveFunction - reported as an engine bug alongside this test.)
// ============================================================================

triSimUntil { GM_LIB_READY }

// -- undercover.sqs established cover ------------------------------------------
triSimUntil { gmUndercover }
triSimUntil { captive player }

// -- baseline heat on the zone nearest the player (= the Camp he stands in) ---
guCamp = gmZoneIndex "Camp"
triAssertGe [guCamp, 0]
guHeat0 = (gmZone guCamp) select 6

// -- the guerrilla "opens fire": the same latch the mission's fired-EH pulls --
gmBreakUndercover "fired"

// -- native alert tick (~5s) latches the break, fires undercoverBroken;
//    undercover.sqs (~2s manager) drops cover and un-captives the player -----
triSimUntil { not gmUndercover }
triSimUntil { not (captive player) }

// -- native heat spike landed on the nearest zone (alertHeatBreak 25;
//    GREEN-zone decay is only 1 per 10s, so >= +20 is safely conservative) ----
triAssertGe [((gmZone guCamp) select 6), guHeat0 + 20]

triEndTest
