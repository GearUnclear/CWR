// ============================================================================
//  Guerrilla Mode deep undercover - the per-observer perception rules, live.
//    Exposure is engine code on the observer's Target record (per-group, not
//    global): weapon in hands reads hostile, a slung rifle reads hostile only
//    from behind or inside the ~20 m notice radius, unarmed reads civilian,
//    and compromise memory is per witness group. This proves, on full CWA
//    data (Abel), against a staged East observer:
//      1. unarmed at 40 m, face to face: clean (gmUndercoverStatus 0, zero
//         witness groups; knowsAbout is ID confidence, deliberately unasserted);
//      2. rifle IN HANDS at 40 m: identified hostile (status 2) and the
//         witness record crosses the ID threshold (knowsAbout >= 1.5);
//      3. gmUndercoverForget + rifle slung on the back, observer in FRONT at
//         60 m: the torso hides the rifle - stays clean over a window;
//      4. same slung rifle with the player's BACK to the observer: the slung
//         long gun is visible - suspicion at least (status >= 1);
//      5. forget again, slung, observer closed to 15 m in FRONT: inside the
//         notice radius nothing stays hidden - compromised (status 2);
//      6. per-group isolation: a second East group parked ~610 m away at the
//         Outpost never learns any of it while group A's compromise stands.
//
//  Staging notes: both observer groups run combat mode BLUE (never fire) with
//  MOVE disabled, and the player is damage-shielded - perception is the only
//  observable under test. `player fire` crashes headless (known engine bug),
//  so the fired-EH chain is out of scope here; the break machinery is pinned
//  by guerrilla_native_undercover. Facing is driven with setDir on both ends
//  (backArcCos -0.2 gives the back arc plenty of margin at 180 degrees).
//  WEAPONONBACK below is the same UI action id the engine's action menu uses
//  (Transport.cpp EnumName table); if headless stance transitions prove
//  unreliable in the integration run, fall back to knowsAbout-band asserts
//  per the plan's risk note.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmUndercover }
triSimUntil { captive player }

// -- staging isolation (same rationale as guerrilla_native_undercover): retire
//    the companion roster before Petra's first spawn tick and sweep any body
//    that beat us to it (twice, bracketing the check-to-spawn race), or she
//    guns down the BLUE observers and the witness records die with them; park
//    the QRF director (loop re-reads GM_QRF_TICK every pass; all zones are
//    still GREEN this early) so phase 2's deliberate RED alert cannot convoy
//    fresh East witnesses into the later settle-to-clean phases. -------------
GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
ruT0 = time
triSimUntil { time > ruT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])

player allowDammage false

// -- the native garrison auto-spawn guarantees the EAST AI center exists
//    before createGroup east (qrf.sqs's idempotent createCenter also ran) ------
ruOut = gmZoneIndex "Outpost"
triAssertGe [ruOut, 0]
triSimUntil { gmGarrisonSpawned ruOut }

// -- strip the loadout BEFORE the observers exist: phase 1 needs a genuine
//    civilian silhouette from the first perception tick ------------------------
removeAllWeapons player

// -- stage on PROBE-VERIFIED DRY OPEN GROUND at [7500,5700], 100 m west of
//    the Camp centre. CAUTION from the first staging attempt: an object-free
//    disc is NOT a land check - the sea east of the Camp is also object-free,
//    and a swimming unit drops its rifle, so the classifier read weapon 0
//    forever. This corridor was verified BOTH ways: getPosASL elevation 9.1
//    at the stage, 11.5 at -40 m, 15.6 at -60 m (sea surface reads 1.09),
//    and nearestObjects [[..],[],25] returned only the player at all three
//    points. Observer sits WEST (-x) up a gentle open slope; facing the
//    observer = setDir 270, back to the observer = setDir 90. -----------------
ruPos = [7500, 5700, 0]
player setPos ruPos
player setDir 270

// -- observer group A: 40 m west, facing the player ----------------------------
ruGrpA = createGroup east
"SoldierEB" createUnit [[(ruPos select 0) - 40, ruPos select 1, 0], ruGrpA, "ruObs = this", 0.5, "PRIVATE"]
triSimUntil { (count units ruGrpA) >= 1 }
ruGrpA setCombatMode "BLUE"
ruObs disableAI "MOVE"
// belt and suspenders: disableAI "MOVE" does not stop combat-AI
// micro-movement (danger-mode bounding closed 40 m to 20 m in diagnostic
// runs and tripped the notice-radius rule mid-phase); stop also
// short-circuits SelectFireWeapon targeting entirely
ruObs stop true
ruObs setDir 90

// -- observer group B: the isolation control, parked at the zone centre
//    FARTHEST from the stage (the Outpost, ~610 m; computed so the stage can
//    move with the terrain) ---------------------------------------------------
ruFar = -1
ruFarD = -1
ruI = 0
while {ruI < gmZoneCount} do {ruD = ((gmZone ruI) select 8) distance ruPos; if (ruD > ruFarD) then {ruFarD = ruD; ruFar = ruI}; ruI = ruI + 1}
triAssertGe [ruFar, 0]
ruVPos = (gmZone ruFar) select 8
ruGrpB = createGroup east
"SoldierEB" createUnit [[ruVPos select 0, ruVPos select 1, 0], ruGrpB, "ruObsB = this", 0.5, "PRIVATE"]
triSimUntil { (count units ruGrpB) >= 1 }
ruGrpB setCombatMode "BLUE"
ruObsB disableAI "MOVE"
ruObsB stop true

// ---- phase 1: UNARMED, 40 m, face to face => civilian ------------------------
// NOTE: no knowsAbout band here. knowsAbout is identification CONFIDENCE, not
// hostility - a clear-LOS observer confidently identifies the unarmed subject
// as a civilian (knowsAbout can legitimately reach 4). The undercover claims
// are status (no group reads suspect/hostile) and witnesses (none compromised).
ruT0 = time
triSimUntil { time > ruT0 + 30 }
triAssertEq [gmUndercoverStatus, 0]
triAssertEq [gmUndercoverWitnesses, 0]

// ---- phase 2: rifle IN HANDS, 40 m => identified hostile ---------------------
player addMagazine "AK47"
player addMagazine "AK47"
player addWeapon "AK47"
player selectWeapon "AK47"
triSimUntil { gmUndercoverStatus == 2 }
triSimUntil { ((leader ruGrpA) knowsAbout player) >= 1.5 }

// ---- phase 3: rifle SLUNG, observer in FRONT at 60 m => clean ----------------
player action ["WEAPONONBACK", player]
ruT0 = time
triSimUntil { time > ruT0 + 5 }
ruObs setPos [(ruPos select 0) - 60, ruPos select 1, 0]
ruObs setDir 90
player setDir 270
gmUndercoverForget
triSimUntil { gmUndercoverStatus == 0 }
ruT0 = time
triSimUntil { time > ruT0 + 25 }
triAssertEq [gmUndercoverStatus, 0]

// ---- phase 4: same slung rifle, player's BACK to the observer => suspect -----
player setDir 90
triSimUntil { gmUndercoverStatus >= 1 }

// ---- phase 5: forget, then slung INSIDE the 20 m notice radius => exposed ----
player setDir 270
gmUndercoverForget
triSimUntil { gmUndercoverStatus == 0 }
ruObs setPos [(ruPos select 0) - 15, ruPos select 1, 0]
ruObs setDir 90
triSimUntil { gmUndercoverStatus == 2 }

// ---- phase 6: per-group isolation - only group A holds the compromise --------
// witnesses == 1 is the precise per-group claim: group A compromised (phase 5),
// group B at the Outpost and its garrison untouched
triAssertEq [gmUndercoverStatus, 2]
triAssertEq [gmUndercoverWitnesses, 1]

player allowDammage true
triEndTest
