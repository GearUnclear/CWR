// ============================================================================
//  LoBo #9 activation case (repair 2026-10-11.3, installed @LoBo only).
//
//  The mission names only LoBoTerror in addOns[], as a mission that places a
//  LoBo Toyota would, so LoBo_airammo (owner of M1Smoker) is not activated.
//  CONTENT-REPAIRS.md says UD then logs one "Access denied" warning and still
//  creates the glow. The probe records what happens, then asserts that the
//  Toyota's owner path created its M1Smoker. Review the screenshot for a
//  visible campfire mesh at the glow position: a denied entry's own members
//  (model "empty") may be invisible, leaving the vanilla Fire model.
// ============================================================================
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
showCinemaBorder false
player allowDammage false
player setCaptive true
wgCount = {wgN = 0; {if ((typeOf _x) == (_this select 1)) then {wgN = wgN + 1}} forEach (nearestObjects [_this select 0, [], 30]); wgN}

// Direct creation by class name, observed before any assertion.
wgG = "M1Smoker" createVehicle [8000, 9560, 0]
logInfo format ["LOBO_R3_GLOW_UNLISTED_CREATE class=%1 null=%2", typeOf wgG, isNull wgG]
wgP = getPosASL wgG
triSetView [(wgP select 0) - 4, (wgP select 2) + 1.5, (wgP select 1) - 4, 4, -1.2, 4]
triSimFrames 5
triScreenshot "unlisted_direct_m1smoker"
deleteVehicle wgG
triSimFrames 3

// Owner path on the Toyota.
wgWreck = "LoBo_Ter_Toy_R" createVehicle [8000, 9620, 0]
triAssertEq [typeOf wgWreck, "LoBo_Ter_Toy_R"]
triSimFrames 10
triAssertEq [[getPos wgWreck, "M1Smoker"] call wgCount, 0]
wgWreck setDammage 1
wgPolls = 0
triSimUntil { call {wgPolls = wgPolls + 1; (([getPos wgWreck, "M1Smoker"] call wgCount) == 1) or (wgPolls > 40)} }
wgGlow = nearestObject [getPos wgWreck, "M1Smoker"]
logInfo format ["LOBO_R3_GLOW_UNLISTED_OWNER wreck=%1 alive=%2 local=%3 glowNull=%4 glow=%5 distance=%6 inflamed=%7 count=%8 polls=%9", typeOf wgWreck, alive wgWreck, local wgWreck, isNull wgGlow, typeOf wgGlow, wgGlow distance wgWreck, inflamed wgGlow, [getPos wgWreck, "M1Smoker"] call wgCount, wgPolls]
wgP = getPosASL wgWreck
triSetView [(wgP select 0) - 9, (wgP select 2) + 3, (wgP select 1) - 9, 9, -2.5, 9]
triSimFrames 3
triScreenshot "unlisted_toyota_wreck_glow"
triAssertEq [alive wgWreck, false]
triAssert [not isNull wgGlow]
triAssertEq [typeOf wgGlow, "M1Smoker"]
triEndTest
