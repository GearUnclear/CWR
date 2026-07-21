// ============================================================================
//  Guerrilla Mode NATIVE parity - undercover break lifecycle (deep-undercover
//  rewrite). Cover no longer breaks globally: exposure is per-observer state
//  on each occupier group's Target record, and the script baseline
//  (gmUndercover + setCaptive true) PERSISTS through a break. This proves, on
//  full CWA data (Abel):
//      1. the mission establishes cover at boot (undercover.sqs:
//         gmUndercover = true + setCaptive true);
//      2. an East observer group that merely KNOWS the unarmed player keeps
//         reading him as clean (gmUndercoverStatus 0);
//      3. gmBreakUndercover "fired" - the EXACT call the mission's fired-EH
//         makes - marks that witness group compromised: gmUndercoverStatus
//         reaches 2 and gmUndercoverWitnesses counts the group;
//      4. the native undercoverBroken event's Heat spike lands on the zone
//         nearest the witness (the Camp, where both stand) by >= +20
//         (alertHeatBreak 25; GREEN decay is only 1 per 10 s);
//      5. NEW lifecycle: gmUndercover stays true and the player stays captive
//         after the break - the old global un-captive reaction is gone
//         (undercover.sqs reacts with an advisory hint only).
//
//  The break enters through the native command the fired-EH uses. (A literal
//  `player fire "AK47CZ"` would be the full chain, but force-firing the real
//  player crashes the engine headless - 0xC0000005 in
//  Man::ProcessMoveFunction - reported as an engine bug alongside the
//  original parity test.)
//
//  The player is stripped of weapons FIRST so the observer builds its
//  knowledge record on a genuine civilian silhouette - the per-observer rules
//  themselves (weapon shown/slung, facing, notice radius) are covered by
//  guerrilla_undercover_rules.test.sqf; this test pins the break machinery.
// ============================================================================

triSimUntil { GM_LIB_READY }

// -- staging isolation: the fixture boots live managers that can shoot up the
//    perception stage. The companion roster (companions.sqs) respawns Petra
//    armed next to the player, and she WILL kill a BLUE hold-fire observer;
//    the QRF director (qrf.sqs) convoys fresh East groups onto the player's
//    last-known position once an alert goes RED. Retire the roster before her
//    first spawn tick, sweep any body that beat us to it (twice, bracketing
//    the check-to-spawn race inside the companion tick), and park the QRF
//    loop (it re-reads GM_QRF_TICK every pass; every zone is still GREEN this
//    early, so it cannot have dispatched yet).
GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
guT0 = time
triSimUntil { time > guT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])

// -- undercover.sqs established cover ------------------------------------------
triSimUntil { gmUndercover }
triSimUntil { captive player }

// -- an unarmed guerrilla is a clean civilian to any observer -------------------
removeAllWeapons player
player allowDammage false

// -- the native garrison auto-spawn guarantees the EAST AI center exists
//    before createGroup east (qrf.sqs's idempotent createCenter also ran) ------
guOut = gmZoneIndex "Outpost"
triAssertGe [guOut, 0]
triSimUntil { gmGarrisonSpawned guOut }

// -- stage on PROBE-VERIFIED DRY OPEN GROUND at [7500,5700], 100 m west of
//    the Camp centre (the zone centre itself and the flats east of it are
//    open SEA - object-free discs alone are not a land check). Verified both
//    ways: getPosASL elevation 9.1 at the stage, 11.5 at the observer point
//    40 m west (sea surface reads 1.09), and nearestObjects [[..],[],25]
//    returned only the player at both points, so the witness Heat spike stays
//    the Camp's (130 m, nearest zone by a >380 m margin). --------------------
guPos = [7500, 5700, 0]
player setPos guPos
player setDir 270

// -- spawn ONE East observer 30 m west, mutually facing.
//    BLUE = never fires, MOVE disabled = holds the stage position. ------------
guGrpE = createGroup east
"SoldierEB" createUnit [[(guPos select 0) - 30, guPos select 1, 0], guGrpE, "guObs = this", 0.5, "PRIVATE"]
triSimUntil { (count units guGrpE) >= 1 }
guGrpE setCombatMode "BLUE"
guObs disableAI "MOVE"
// belt and suspenders: disableAI "MOVE" does not stop combat-AI
// micro-movement (danger-mode bounding closed 40 m to 20 m in diagnostic
// runs); stop also short-circuits SelectFireWeapon targeting entirely
guObs stop true
guObs setDir 90

// -- the observer holds a live knowledge record of the (civilian) player -------
triSimUntil { (guGrpE knowsAbout player) > 0 }

// -- still clean: unarmed + no compromise memory => UCCivil for every group ----
triAssertEq [gmUndercoverStatus, 0]

// -- baseline heat on the zone nearest the WITNESS - the native spike lands
//    there; computed rather than hardcoded so the stage can move with the
//    terrain --------------------------------------------------------------------
guZone = -1
guBestD = 999999
guI = 0
while {guI < gmZoneCount} do {guD = ((gmZone guI) select 8) distance (getPos guObs); if (guD < guBestD) then {guBestD = guD; guZone = guI}; guI = guI + 1}
triAssertGe [guZone, 0]
guHeat0 = (gmZone guZone) select 6

// -- the guerrilla "opens fire": the same latch the mission's fired-EH pulls ---
gmBreakUndercover "fired"

// -- every group that currently knows the player is marked compromised ---------
triSimUntil { gmUndercoverStatus == 2 }
triAssertGe [gmUndercoverWitnesses, 1]

// -- native heat spike lands on the next alert tick (~5 s) ---------------------
triSimUntil { ((gmZone guZone) select 6) >= guHeat0 + 20 }

// -- NEW lifecycle: the script baseline SURVIVES the break ---------------------
triAssert [gmUndercover]
triAssert [captive player]

player allowDammage true
triEndTest
