// ============================================================================
//  LoBo #9 runtime smoke probe (repair 2026-10-11.3, installed @LoBo only).
//
//  Repair .3 defines M1Smoker, M60Smoker, Smoker and KioGlowFire in
//  LoBo_airammo as LoBo_WreckGlow, an invisible vanilla Fire. This probe
//  checks that (1) each class creates a real object, and (2) the owner path
//  of three installed wreck fire.sqs scripts creates its glow when a local,
//  empty vehicle dies through its native killed handler: LoBoTer (Toyota,
//  M1Smoker), LoBoMerkava2 (M60Smoker) and LoBoEghelo (Mi-8, KioGlowFire).
//  The mission activates LoBo_airammo; lobo_wreck_glow_unlisted covers a
//  mission that does not. Screenshots need visual review. Burn damage, the
//  "Put out fire" action and the 150 s cleanup are not covered here.
// ============================================================================
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
showCinemaBorder false
player allowDammage false
player setCaptive true
wgCount = {wgN = 0; {if ((typeOf _x) == (_this select 1)) then {wgN = wgN + 1}} forEach (nearestObjects [_this select 0, [], 30]); wgN}

// 1. Each glow class creates a real object by class name.
wgG = "M1Smoker" createVehicle [7940, 9560, 0]
triAssert [not isNull wgG]
triAssertEq [typeOf wgG, "M1Smoker"]
logInfo format ["LOBO_R3_GLOW_CREATE class=%1 null=%2 inflamed=%3", typeOf wgG, isNull wgG, inflamed wgG]
deleteVehicle wgG
wgG = "M60Smoker" createVehicle [7940, 9560, 0]
triAssert [not isNull wgG]
triAssertEq [typeOf wgG, "M60Smoker"]
logInfo format ["LOBO_R3_GLOW_CREATE class=%1 null=%2 inflamed=%3", typeOf wgG, isNull wgG, inflamed wgG]
deleteVehicle wgG
wgG = "Smoker" createVehicle [7940, 9560, 0]
triAssert [not isNull wgG]
triAssertEq [typeOf wgG, "Smoker"]
logInfo format ["LOBO_R3_GLOW_CREATE class=%1 null=%2 inflamed=%3", typeOf wgG, isNull wgG, inflamed wgG]
deleteVehicle wgG
wgG = "KioGlowFire" createVehicle [7940, 9560, 0]
triAssert [not isNull wgG]
triAssertEq [typeOf wgG, "KioGlowFire"]
logInfo format ["LOBO_R3_GLOW_CREATE class=%1 null=%2 inflamed=%3", typeOf wgG, isNull wgG, inflamed wgG]
deleteVehicle wgG
triSimFrames 3

// 2. Owner path: the installed killed handler runs fire.sqs on a local wreck.
// --- LoBoTer Toyota: M1Smoker ---
wgWreck = "LoBo_Ter_Toy_R" createVehicle [8000, 9620, 0]
triAssertEq [typeOf wgWreck, "LoBo_Ter_Toy_R"]
triSimFrames 10
triAssertEq [[getPos wgWreck, "M1Smoker"] call wgCount, 0]
wgWreck setDammage 1
triSimUntil { ([getPos wgWreck, "M1Smoker"] call wgCount) == 1 }
wgGlow = nearestObject [getPos wgWreck, "M1Smoker"]
triAssert [not isNull wgGlow]
logInfo format ["LOBO_R3_GLOW_OWNER wreck=%1 alive=%2 local=%3 glow=%4 distance=%5 inflamed=%6 count=%7", typeOf wgWreck, alive wgWreck, local wgWreck, typeOf wgGlow, wgGlow distance wgWreck, inflamed wgGlow, [getPos wgWreck, "M1Smoker"] call wgCount]
triAssertEq [alive wgWreck, false]
triAssertLe [wgGlow distance wgWreck, 5]
triSimFrames 120
wgP = getPosASL wgWreck
triSetView [(wgP select 0) - 9, (wgP select 2) + 3, (wgP select 1) - 9, 9, -2.5, 9]
triSimFrames 3
triScreenshot "toyota_wreck_glow"

// --- LoBoMerkava2: M60Smoker ---
wgWreck = "LoBoMerkava2" createVehicle [8000, 9720, 0]
triAssertEq [typeOf wgWreck, "LoBoMerkava2"]
triSimFrames 10
triAssertEq [[getPos wgWreck, "M60Smoker"] call wgCount, 0]
wgWreck setDammage 1
triSimUntil { ([getPos wgWreck, "M60Smoker"] call wgCount) == 1 }
wgGlow = nearestObject [getPos wgWreck, "M60Smoker"]
triAssert [not isNull wgGlow]
logInfo format ["LOBO_R3_GLOW_OWNER wreck=%1 alive=%2 local=%3 glow=%4 distance=%5 inflamed=%6 count=%7", typeOf wgWreck, alive wgWreck, local wgWreck, typeOf wgGlow, wgGlow distance wgWreck, inflamed wgGlow, [getPos wgWreck, "M60Smoker"] call wgCount]
triAssertEq [alive wgWreck, false]
triAssertLe [wgGlow distance wgWreck, 5]
triSimFrames 120
wgP = getPosASL wgWreck
triSetView [(wgP select 0) - 11, (wgP select 2) + 4, (wgP select 1) - 11, 11, -3, 11]
triSimFrames 3
triScreenshot "merkava2_wreck_glow"

// --- LoBoEghelo Mi-8: KioGlowFire ---
wgWreck = "LoBo_Mi8_egy" createVehicle [8000, 9820, 0]
triAssertEq [typeOf wgWreck, "LoBo_Mi8_egy"]
triSimFrames 10
triAssertEq [[getPos wgWreck, "KioGlowFire"] call wgCount, 0]
wgWreck setDammage 1
triSimUntil { ([getPos wgWreck, "KioGlowFire"] call wgCount) == 1 }
wgGlow = nearestObject [getPos wgWreck, "KioGlowFire"]
triAssert [not isNull wgGlow]
logInfo format ["LOBO_R3_GLOW_OWNER wreck=%1 alive=%2 local=%3 glow=%4 distance=%5 inflamed=%6 count=%7", typeOf wgWreck, alive wgWreck, local wgWreck, typeOf wgGlow, wgGlow distance wgWreck, inflamed wgGlow, [getPos wgWreck, "KioGlowFire"] call wgCount]
triAssertEq [alive wgWreck, false]
triAssertLe [wgGlow distance wgWreck, 5]
triSimFrames 120
wgP = getPosASL wgWreck
triSetView [(wgP select 0) - 14, (wgP select 2) + 5, (wgP select 1) - 14, 14, -4, 14]
triSimFrames 3
triScreenshot "mi8_wreck_glow"
logInfo format ["LOBO_R3_GLOW_DONE player_damage=%1 player_distance_to_last=%2", damage player, player distance wgWreck]
triEndTest
