// ============================================================================
//  Guerrilla Mode - the vanilla civilian-class disguise ladder, live
//  (issue #25 M3.3: "the missing fire-exposure integration test").
//
//  A SoldierGFakeC (": Civilian", side=2, accuracy=2 - BIS's own
//  plainclothes fighter) createUnit'd into a GUER-side group exercises the
//  UNMODIFIED side-resolve ladder (Target.cpp; the undercover layer is
//  player-only and never applies to an AI subject). Three claims, each
//  load-bearing for the whole outfit/disguise axis:
//
//    1. DISTANCE DISGUISE: at ~150 m a revealed record reads side CIV -
//       GetType at low accuracy walks up to the Civilian parent and
//       GetTargetSide below sideAccuracy 1.5 returns that coarse type's
//       side (VehicleAIDiag.cpp GetType/GetTargetSide).
//    2. CLOSE-RANGE ID: once the observer's sideAccuracy crosses 1.5
//       (clear LOS, short range) the record resolves the true instance
//       side - resistance.
//    3. FIRE EXPOSURE (the free vanilla break): a SECOND observer group
//       still reading CIV at range flips to resistance the moment the
//       disguised fighter fires on it - the fired-at branch raises side
//       knowledge to the observer's knowledge of what was shot at, and an
//       observer's record of ITSELF is pinned at accuracy 4
//       (Target.cpp fired-at branch + TargetSeesItself). Asserted as
//       "stays resistance over a settle window", not literal permanence -
//       FadingSideAccuracy decays over minutes once observation stops.
//
//  Staging follows guerrilla_undercover_rules: probe-verified dry ground at
//  [7500,5700], BLUE + disableAI MOVE + stop for every staged unit, the
//  companion roster retired and the QRF director parked. The player (captive
//  via undercover.sqs) is damage-shielded and irrelevant to every record
//  under test.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmUndercover }
triSimUntil { captive player }

// -- staging isolation (rationale in guerrilla_undercover_rules): no Petra,
//    no QRF convoys; sweep twice bracketing the companion spawn race --------
GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
fkT0 = time
triSimUntil { time > fkT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])

player allowDammage false

// -- the native garrison spawn guarantees the EAST center exists -------------
fkOut = gmZoneIndex "Outpost"
triAssertGe [fkOut, 0]
triSimUntil { gmGarrisonSpawned fkOut }

// -- stage: the disguised fighter on the dry corridor, player parked well
//    EAST behind it and out of every line of fire ----------------------------
fkPos = [7500, 5700, 0]
player setPos [(fkPos select 0) + 120, fkPos select 1, 0]

fkGrp = createGroup resistance
"SoldierGFakeC" createUnit [fkPos, fkGrp, "fkFake = this", 0.5, "PRIVATE"]
triSimUntil { (count units fkGrp) >= 1 }
triAssertEq [(typeOf fkFake), "SoldierGFakeC"]
// the instance fights as resistance regardless of what observers read
triAssertEq [(format ["%1", side fkFake]), "GUER"]
fkFake allowDammage false
// hold fire and hold position until phase 3 explicitly unleashes it
fkGrp setCombatMode "BLUE"
fkFake disableAI "MOVE"
fkFake stop true
fkFake setDir 270

// -- observer A (phases 1-2): 150 m west, facing the fighter; BLUE + stopped
//    (belt and suspenders per the rules test: disableAI MOVE alone does not
//    stop danger-mode bounding) ----------------------------------------------
fkGrpA = createGroup east
"SoldierEB" createUnit [[(fkPos select 0) - 150, fkPos select 1, 0], fkGrpA, "fkObsA = this", 0.5, "PRIVATE"]
triSimUntil { (count units fkGrpA) >= 1 }
fkGrpA setCombatMode "BLUE"
fkObsA disableAI "MOVE"
fkObsA stop true
fkObsA setDir 90
fkObsA allowDammage false

// -- observer B (phase 3): a SEPARATE group 150 m WSW, warming its own LOS
//    from the start so the fire-exposure read is not gated on cold optics ----
fkGrpB = createGroup east
"SoldierEB" createUnit [[(fkPos select 0) - 150, (fkPos select 1) - 30, 0], fkGrpB, "fkObsB = this", 0.5, "PRIVATE"]
triSimUntil { (count units fkGrpB) >= 1 }
fkGrpB setCombatMode "BLUE"
fkObsB disableAI "MOVE"
fkObsB stop true
fkObsB setDir 75
fkObsB allowDammage false

// -- knownTargets probe: side string of grp's record of tgt, "NONE" if no
//    known record ([grp, tgt] call fkSideOf) ---------------------------------
fkSideOf = {fkKT = knownTargets (_this select 0); fkTgt = _this select 1; fkR = "NONE"; fkI = 0; while {fkI < (count fkKT)} do {fkE = fkKT select fkI; if ((fkE select 0) == fkTgt) then {fkR = format ["%1", fkE select 1]}; fkI = fkI + 1}; fkR}

// ---- phase 1: DISTANCE DISGUISE --------------------------------------------
// seed both records (reveal = accuracy 1 / sideAccuracy 1: known, not
// identified - GetTargetSide(1) walks the coarse type to the Civilian
// parent), then hold a window: both groups keep reading CIV at 150 m
fkGrpA reveal fkFake
fkGrpB reveal fkFake
triSimUntil { ([fkGrpA, fkFake] call fkSideOf) == "CIV" }
fkT0 = time
triSimUntil { time > fkT0 + 30 }
triAssertEq [([fkGrpA, fkFake] call fkSideOf), "CIV"]
triAssertEq [([fkGrpB, fkFake] call fkSideOf), "CIV"]
// identification confidence stayed under the 1.35 suspicion band at range
triAssertLt [((leader fkGrpA) knowsAbout fkFake), 1.35]

// ---- phase 2: CLOSE-RANGE ID -----------------------------------------------
// move the warmed observer A inside conversational range: sideAccuracy
// builds past 1.5 and the record resolves the true instance side
fkObsA setPos [(fkPos select 0) - 15, fkPos select 1, 0]
fkObsA setDir 90
triSimUntil { ((leader fkGrpA) knowsAbout fkFake) >= 1.5 }
triSimUntil { ([fkGrpA, fkFake] call fkSideOf) == "GUER" }
// group isolation: B, still at 150 m, keeps the civilian read
triAssertEq [([fkGrpB, fkFake] call fkSideOf), "CIV"]

// ---- phase 3: FIRE EXPOSURE ------------------------------------------------
// the disguised fighter opens fire ON observer B. B's record of itself is
// pinned at accuracy 4, so the fired-at branch raises B's side knowledge of
// the shooter to 4 -> the record flips to the real side without B ever
// closing the distance.
fkGrp reveal fkObsB
fkGrp setCombatMode "RED"
fkFake stop false
fkFake doFire fkObsB
triSimUntil { ([fkGrpB, fkFake] call fkSideOf) == "GUER" }

// the exposure holds across a settle window while the engagement stands
fkT0 = time
triSimUntil { time > fkT0 + 20 }
triAssertEq [([fkGrpB, fkFake] call fkSideOf), "GUER"]

player allowDammage true
triEndTest
