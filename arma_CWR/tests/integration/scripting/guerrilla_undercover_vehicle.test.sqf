// ============================================================================
//  Guerrilla Mode deep undercover - the per-observer VEHICLE policy, live.
//    A captive subject aboard a vehicle resolves per occupier group on the
//    vehicle's Target record (EntityAI::TrackTargets -> ResolvePerceivedSide
//    -> ResolvePerceivedSideVehicle), exactly like the on-foot person rule but
//    keyed on the vehicle's typical side, the checkpoint notice radius (20 m)
//    and the getaway window (a group that just identified the PERSON engages
//    the car he fled into). This proves, on full CWA data (Abel), against one
//    staged East (occupier) observer:
//      A. UNARMED player driving a CIVILIAN car (Skoda) at checkpoint range
//         (15 m, LOS): anonymous by policy - stays clean (status 0, zero
//         witnesses). A civilian plate is never suspicious on its own;
//      B. player driving a STOLEN OCCUPIER MILITARY vehicle (the EAST "UAZ",
//         Abel's WL1 QRF hull): at long range (60 m) the disguise holds - NOT
//         exposed (status < 2); close the observer to checkpoint range (15 m)
//         and the theft is made - status 2, >= 1 witness group, and the
//         campaign-first compromise spikes alertHeatBreak (25) Heat on the
//         witness's nearest zone;
//      C. WITNESSED GETAWAY: identify the PERSON first (rifle in hands at 15 m
//         -> status 2), then within the 10 s board-witness window board a
//         CIVILIAN car in that group's view. The getaway rule marks the CAR's
//         record compromised even though a civilian hull is otherwise
//         anonymous (contrast phase A, same car, same range, clean) - observed
//         as a fresh witness Heat spike (undercoverHeatWitness 8) landing on
//         top of the person's already-settled spike. status stays 2.
//
//  Why the staging looks like this:
//    * ONE observer group, reused across all three phases and moved with
//      setPos (a fresh spawn eats ~65 s of headless LOS warmup; warm it once
//      on the on-foot player, then drive it near/far). gmUndercoverForget
//      between phases clears every compromise record AND the campaign
//      first-ever latch, so B's and C's spikes each read as first-ever/witness
//      cleanly; it does NOT touch zone Heat, so every phase captures its own
//      Heat baseline and asserts a delta.
//    * The observer runs combat mode BLUE (never fires) with MOVE disabled and
//      `stop true` (disableAI "MOVE" alone lets danger-mode bounding creep the
//      40 m gap down to the 20 m notice radius mid-phase); the player is
//      damage-shielded. Perception is the only observable under test.
//    * Companion roster retired before Petra's first spawn tick (she respawns
//      armed and guns down the BLUE observer, and the witness record dies with
//      her); QRF director parked (a RED alert would convoy fresh East witnesses
//      onto the stage). Same isolation preamble as the two sibling undercover
//      tests.
//    * Stage on PROBE-VERIFIED DRY OPEN GROUND at [7500,5700], 100 m west of
//      the Camp centre. The Camp centre itself and the flats east are OPEN SEA
//      (an object-free disc is NOT a land check - a swimming unit drops its
//      rifle). The west corridor is dry: getPosASL elevation ~9 at the stage,
//      ~10 at 15 m west, ~15.6 at 60 m west (sea surface reads ~1.09). The car
//      and UAZ spawn at the dry stage; the observer moves along the west slope.
//    * The vehicle cache (_subjectVehicle) syncs one frame AFTER moveInDriver
//      (SyncCaches runs in AlertMachine::Simulate, after PerformAI), so every
//      mount waits for `(vehicle player) != player` plus a short settle before
//      any assert. triAssert/triRefute with a RAW Bool hangs the harness, so
//      claims are scalar (triAssertEq/Ge/Lt) or a triSimUntil condition.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmUndercover }
triSimUntil { captive player }

// -- staging isolation (see the banner + the two sibling tests) ----------------
GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
vuT0 = time
triSimUntil { time > vuT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])

player allowDammage false

// -- the native garrison auto-spawn guarantees the EAST AI center exists before
//    createGroup east (qrf.sqs's idempotent createCenter also ran) -------------
vuOut = gmZoneIndex "Outpost"
triAssertGe [vuOut, 0]
triSimUntil { gmGarrisonSpawned vuOut }

// -- classes: the civilian hull and the occupier's own military hull. UAZ is
//    Abel's EAST WL1 QRF vehicle (occupier side => UCVOccupierMilitary); Skoda
//    is a civilian car (typical side Civilian => UCVCivilian). Guard both so a
//    package swap fails loud, not silently mis-typed ---------------------------
vuCiv = "Skoda"
vuMil = "UAZ"
triAssert [gmClassExists vuCiv]
triAssert [gmClassExists vuMil]

// -- strip the loadout BEFORE the observer exists so it builds its record on a
//    genuine civilian silhouette from the first perception tick ----------------
removeAllWeapons player

// -- stage + geometry (all probe-verified dry, see banner) ---------------------
vuPos = [7500, 5700, 0]
vuClose = [7485, 5700, 0]
vuFar = [7440, 5700, 0]
player setPos vuPos
player setDir 270

// -- one observer, spawned at checkpoint range, mutually facing ----------------
vuGrp = createGroup east
"SoldierEB" createUnit [vuClose, vuGrp, "vuObs = this", 0.5, "PRIVATE"]
triSimUntil { (count units vuGrp) >= 1 }
vuGrp setCombatMode "BLUE"
vuObs disableAI "MOVE"
vuObs stop true
vuObs setDir 90

// -- warm the observer's LOS on the on-foot, unarmed (clean) player ------------
triSimUntil { (leader vuGrp) knowsAbout player > 0 }

// -- Heat baseline zone: nearest to the checkpoint-range observer position. The
//    per-observer compromise spike lands on the zone nearest the WITNESS, and
//    every compromise in this test happens at vuClose; computed, not hardcoded,
//    so the stage can move with the terrain ------------------------------------
vuZone = -1
vuBestD = 999999
vuI = 0
while {vuI < gmZoneCount} do {vuD = ((gmZone vuI) select 8) distance vuClose; if (vuD < vuBestD) then {vuBestD = vuD; vuZone = vuI}; vuI = vuI + 1}
triAssertGe [vuZone, 0]

// ============================================================================
//  PHASE A - CIVILIAN car, checkpoint range, unarmed => anonymous (clean)
// ============================================================================
vuCarA = vuCiv createVehicle vuPos
player moveInDriver vuCarA
triSimUntil { (vehicle player) != player }
vuT0 = time
triSimUntil { time > vuT0 + 4 }
// a civilian hull inside the notice radius is still just a civilian car
vuT0 = time
triSimUntil { time > vuT0 + 26 }
triAssertEq [gmUndercoverStatus, 0]
triAssertEq [gmUndercoverWitnesses, 0]
// dismount by deleting the hull (the driver is set down on the dry stage)
deleteVehicle vuCarA
triSimUntil { (vehicle player) == player }

// ============================================================================
//  PHASE B - STOLEN OCCUPIER MILITARY vehicle: holds at range, made up close
// ============================================================================
gmUndercoverForget
triSimUntil { gmUndercoverStatus == 0 }
vuUaz = vuMil createVehicle vuPos
player moveInDriver vuUaz
triSimUntil { (vehicle player) != player }
vuT0 = time
triSimUntil { time > vuT0 + 4 }

// -- pull the observer out to long range: the theft is not readable at 60 m ----
vuObs setPos vuFar
vuObs setDir 90
// baseline Heat before any compromise (still not exposed at long range)
vuHeatB0 = (gmZone vuZone) select 6
vuT0 = time
triSimUntil { time > vuT0 + 26 }
triAssertLt [gmUndercoverStatus, 2]

// -- close to checkpoint range: the stolen military hull is made ---------------
vuObs setPos vuClose
vuObs setDir 90
triSimUntil { gmUndercoverStatus == 2 }
triAssertGe [gmUndercoverWitnesses, 1]
// campaign-first compromise (forget reset the latch) => the big alarm
triSimUntil { ((gmZone vuZone) select 6) >= vuHeatB0 + 20 }

// -- back on foot for the getaway phase ----------------------------------------
deleteVehicle vuUaz
triSimUntil { (vehicle player) == player }

// ============================================================================
//  PHASE C - WITNESSED GETAWAY: the group that made the PERSON engages the
//  civilian car he boards, even though that hull is otherwise anonymous
// ============================================================================
gmUndercoverForget
triSimUntil { gmUndercoverStatus == 0 }

// -- identify the person: rifle in hands at 15 m => exposed (first-ever) --------
player setDir 270
player addMagazine "AK47"
player addMagazine "AK47"
player addWeapon "AK47"
player selectWeapon "AK47"
triSimUntil { gmUndercoverStatus == 2 }

// -- let the person's own spike settle, then capture the getaway baseline. The
//    zone alert STATE is garrison-driven (this standalone observer is not a
//    garrison unit), so once the person spike has landed nothing but a NEW
//    compromise edge can raise this zone's Heat further -------------------------
vuT0 = time
triSimUntil { time > vuT0 + 16 }
vuHeatC0 = (gmZone vuZone) select 6

// -- board the CIVILIAN car inside the 10 s board-witness window. Weapon goes on
//    the back first so the mount is a clean board, not a firing pose; the person
//    record is already compromised and its lastSeen is fresh -------------------
player action ["WEAPONONBACK", player]
vuCarC = vuCiv createVehicle vuPos
player moveInDriver vuCarC
triSimUntil { (vehicle player) != player }

// -- the getaway rule marks the CAR's record compromised - a fresh witness Heat
//    spike (undercoverHeatWitness 8) on top of the settled person spike. Phase A
//    proved the same hull at the same range is clean WITHOUT a prior person ID,
//    so this delta is the getaway rule, not the plate ------------------------
triSimUntil { ((gmZone vuZone) select 6) >= vuHeatC0 + 5 }
triAssertEq [gmUndercoverStatus, 2]
triAssertGe [gmUndercoverWitnesses, 1]

player allowDammage true
triEndTest
