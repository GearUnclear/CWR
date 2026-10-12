// ============================================================================
//  LoBo #8 runtime smoke probe (repair 2026-10-11.3, installed @LoBo only).
//
//  Tail-number scripts on the F-15 (LoBoF15 scr\number.sqs), F-16 IAF
//  (LoBoF16 scr\number.sqs), F-16 EAF (scr\number2.sqs), F-4 (LoBo_F4
//  scripts\number.sqs) and IL-28 (LoBoIL28 script\number.sqs, started by
//  common_init.sqs). Each class is spawned twice, empty, and its installed
//  Init handler runs the script; nothing here sets a texture. The runner aborts
//  on any script error (--autotest), and the toml runs at debug log level so an
//  unresolved goto ("Unknown label") would be logged. There is no texture
//  getter, so the digits themselves need visual review of the screenshots:
//  two copies with different digits show the script painted them.
// ============================================================================
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
setViewDistance 1500
showCinemaBorder false
player allowDammage false
player setCaptive true
tnY = 9500

// --- F-15C IAF ---
tnPlane = "LoBo_F15C_IAF_grey" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "LoBo_F15C_IAF_grey"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=a pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 22, (tnP select 2) + 2.5, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "f15c_a_profile"
triSetView [(tnP select 0) - 7, (tnP select 2) + 1.5, (tnP select 1) + 5, 7, -0.6, 0]
triSimFrames 3
triScreenshot "f15c_a_nose"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2.5, (tnP select 1) - 5, 8, -0.3, 0]
triSimFrames 3
triScreenshot "f15c_a_tail"
tnY = tnY + 50
tnPlane = "LoBo_F15C_IAF_grey" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "LoBo_F15C_IAF_grey"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=b pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 22, (tnP select 2) + 2.5, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "f15c_b_profile"
triSetView [(tnP select 0) - 7, (tnP select 2) + 1.5, (tnP select 1) + 5, 7, -0.6, 0]
triSimFrames 3
triScreenshot "f15c_b_nose"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2.5, (tnP select 1) - 5, 8, -0.3, 0]
triSimFrames 3
triScreenshot "f15c_b_tail"

// --- F-16 IAF (number.sqs) ---
tnY = tnY + 50
tnPlane = "Lobo_F16_IAF_AA" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "Lobo_F16_IAF_AA"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=a pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 20, (tnP select 2) + 2.5, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "f16iaf_a_profile"
triSetView [(tnP select 0) - 7, (tnP select 2) + 1.5, (tnP select 1) + 4, 7, -0.6, 0]
triSimFrames 3
triScreenshot "f16iaf_a_nose"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2.5, (tnP select 1) - 5, 8, -0.3, 0]
triSimFrames 3
triScreenshot "f16iaf_a_tail"
tnY = tnY + 50
tnPlane = "Lobo_F16_IAF_AA" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "Lobo_F16_IAF_AA"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=b pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 20, (tnP select 2) + 2.5, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "f16iaf_b_profile"
triSetView [(tnP select 0) - 7, (tnP select 2) + 1.5, (tnP select 1) + 4, 7, -0.6, 0]
triSimFrames 3
triScreenshot "f16iaf_b_nose"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2.5, (tnP select 1) - 5, 8, -0.3, 0]
triSimFrames 3
triScreenshot "f16iaf_b_tail"

// --- F-16 EAF (number2.sqs) ---
tnY = tnY + 50
tnPlane = "Lobo_F16_EAF_AA" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "Lobo_F16_EAF_AA"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=a pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 20, (tnP select 2) + 2.5, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "f16eaf_a_profile"
triSetView [(tnP select 0) - 7, (tnP select 2) + 1.5, (tnP select 1) + 4, 7, -0.6, 0]
triSimFrames 3
triScreenshot "f16eaf_a_nose"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2.5, (tnP select 1) - 5, 8, -0.3, 0]
triSimFrames 3
triScreenshot "f16eaf_a_tail"
tnY = tnY + 50
tnPlane = "Lobo_F16_EAF_AA" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "Lobo_F16_EAF_AA"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=b pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 20, (tnP select 2) + 2.5, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "f16eaf_b_profile"
triSetView [(tnP select 0) - 7, (tnP select 2) + 1.5, (tnP select 1) + 4, 7, -0.6, 0]
triSimFrames 3
triScreenshot "f16eaf_b_nose"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2.5, (tnP select 1) - 5, 8, -0.3, 0]
triSimFrames 3
triScreenshot "f16eaf_b_tail"

// --- F-4E WW ---
tnY = tnY + 50
tnPlane = "LoBo_F4_WW_IAF" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "LoBo_F4_WW_IAF"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=a pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 22, (tnP select 2) + 2.5, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "f4_a_profile"
triSetView [(tnP select 0) - 7, (tnP select 2) + 1.5, (tnP select 1) + 5, 7, -0.6, 0]
triSimFrames 3
triScreenshot "f4_a_nose"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2.5, (tnP select 1) - 6, 8, -0.3, 0]
triSimFrames 3
triScreenshot "f4_a_tail"
tnY = tnY + 50
tnPlane = "LoBo_F4_WW_IAF" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "LoBo_F4_WW_IAF"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=b pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 22, (tnP select 2) + 2.5, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "f4_b_profile"
triSetView [(tnP select 0) - 7, (tnP select 2) + 1.5, (tnP select 1) + 5, 7, -0.6, 0]
triSimFrames 3
triScreenshot "f4_b_nose"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2.5, (tnP select 1) - 6, 8, -0.3, 0]
triSimFrames 3
triScreenshot "f4_b_tail"

// --- IL-28 ---
tnY = tnY + 50
tnPlane = "LoBo_IL28_egy" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "LoBo_IL28_egy"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=a pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 26, (tnP select 2) + 3, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "il28_a_profile"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2, (tnP select 1) + 6, 8, -0.6, 0]
triSimFrames 3
triScreenshot "il28_a_nose"
triSetView [(tnP select 0) - 9, (tnP select 2) + 3, (tnP select 1) - 7, 9, -0.3, 0]
triSimFrames 3
triScreenshot "il28_a_tail"
tnY = tnY + 50
tnPlane = "LoBo_IL28_egy" createVehicle [8000, tnY, 0]
triAssertEq [typeOf tnPlane, "LoBo_IL28_egy"]
tnPlane setDir 0
tnPlane setPos [8000, tnY, 0]
triSimFrames 60
tnP = getPosASL tnPlane
logInfo format ["LOBO_R3_NUMBERS spawned=%1 copy=b pos=%2 alive=%3", typeOf tnPlane, tnP, alive tnPlane]
triSetView [(tnP select 0) - 26, (tnP select 2) + 3, tnP select 1, 1, -0.08, 0]
triSimFrames 3
triScreenshot "il28_b_profile"
triSetView [(tnP select 0) - 8, (tnP select 2) + 2, (tnP select 1) + 6, 8, -0.6, 0]
triSimFrames 3
triScreenshot "il28_b_nose"
triSetView [(tnP select 0) - 9, (tnP select 2) + 3, (tnP select 1) - 7, 9, -0.3, 0]
triSimFrames 3
triScreenshot "il28_b_tail"
logInfo "LOBO_R3_NUMBERS_DONE"
triEndTest
