// ============================================================================
//  Shared body of the LoBo #14 reference probe (repair 2026-10-11.4).
//  Included by lobo_missing_refs.test.sqf (UD engine, R2) and by
//  lobo_official_smoke.test.sqf (official Remaster exe, R5 S2). Not a test on
//  its own: the file name has no ".test." part, so a directory run skips it.
//
//  Every object is created from the script after the boot; the mission places
//  only stock SoldierWB plus the r4East pilot. Each spawn loads its shape and
//  type, which is when the engine logs a missing texture
//  ("GL33: Cannot load texture <name>", once per face load, every LOD) or a
//  missing sound ("FileCache: file '<name>' not found", "Load failed").
//  The log is checked afterwards by repair4/check_logs.py; the screenshots
//  need visual review.
//
//  Expects the caller to have set rfView, rfHold, rfWound and rfHits.
//  rfView [object, bearing, distance, camera height, aim height]: camera on the
//  compass bearing from the object (0 north, 90 east), looking back at it.
// ============================================================================

// --- Toyota Hilux (LoBoTer lobo_terhilux.p3d, LOD 3.0 and 5.65 repointed) ---
// Facing east on the runway line x=8000 (flat), camera south of it looking north.
rfV = "LoBo_Ter_Toy_R" createVehicle [8000, 9800, 0]
triAssertEq [typeOf rfV, "LoBo_Ter_Toy_R"]
call {rfV setDir 90; rfV setPos [8000, 9800, 0]}
triSimFrames 30
logInfo format ["LOBO_R4_REFS spawned=%1 pos=%2 alive=%3 lodBias=%4", typeOf rfV, getPosASL rfV, alive rfV, triGetObjectLodBias]
triSetView ([rfV, 180, 8, 1.2, 0] call rfView)
triSimFrames 5
triScreenshot "hilux_side_008m"
triSetView ([rfV, 180, 35, 1.5, 0] call rfView)
triSimFrames 5
triScreenshot "hilux_side_035m"
triSetView ([rfV, 180, 70, 1.8, 0] call rfView)
triSimFrames 5
triScreenshot "hilux_side_070m"
triSetView ([rfV, 180, 120, 2.0, 0] call rfView)
triSimFrames 5
triScreenshot "hilux_side_120m"
triClearView
deleteVehicle rfV
triSimFrames 5

// --- Toyota Hilux MG (LoBoTer lobo_terhiluxmg.p3d) ---
rfV = "LoBo_Ter_ToyMG_R" createVehicle [8000, 9800, 0]
triAssertEq [typeOf rfV, "LoBo_Ter_ToyMG_R"]
call {rfV setDir 90; rfV setPos [8000, 9800, 0]}
triSimFrames 30
logInfo format ["LOBO_R4_REFS spawned=%1 pos=%2 alive=%3 lodBias=%4", typeOf rfV, getPosASL rfV, alive rfV, triGetObjectLodBias]
triSetView ([rfV, 180, 8, 1.2, 0] call rfView)
triSimFrames 5
triScreenshot "hiluxmg_side_008m"
triSetView ([rfV, 180, 35, 1.5, 0] call rfView)
triSimFrames 5
triScreenshot "hiluxmg_side_035m"
triSetView ([rfV, 180, 70, 1.8, 0] call rfView)
triSimFrames 5
triScreenshot "hiluxmg_side_070m"
triSetView ([rfV, 180, 120, 2.0, 0] call rfView)
triSimFrames 5
triScreenshot "hiluxmg_side_120m"
triClearView
deleteVehicle rfV
triSimFrames 5

// --- EAF pilot (LoBo_Pilot lobo_epilot.p3d, LOD 1.0 and 5.0 repointed) ---
call {rfU = objNull; "LoBo_Egypt_Pilot" createUnit [[8000, 9800, 0], group r4East, "rfU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rfU, "LoBo_Egypt_Pilot"]
[rfU] call rfHold
triSimFrames 60
logInfo format ["LOBO_R4_REFS spawned=%1 pos=%2 dir=%3 alive=%4 lodBias=%5", typeOf rfU, getPosASL rfU, getDir rfU, alive rfU, triGetObjectLodBias]
triSetView ([rfU, (getDir rfU) + 180, 5, 1.3, 1.0] call rfView)
triSimFrames 5
triScreenshot "eaf_pilot_back_005m"
triSetView ([rfU, (getDir rfU) + 180, 40, 1.6, 1.0] call rfView)
triSimFrames 5
triScreenshot "eaf_pilot_back_040m"
// Wound textures: Man::WoundsAnimation draws a body/hands/legs wound only at a
// hit of 0.7 or more (head 0.8). setDammage 0.6 alone sets every hit to 0.6 and
// shows none, so the per-part hits are raised; total damage stays 0.5.
[rfU] call rfWound
triSimFrames 30
logInfo format ["LOBO_R4_REFS wounded=%1 damage=%2 alive=%3 hits[hlava,telo,ruce,nohy]=%4", typeOf rfU, damage rfU, alive rfU, [rfU] call rfHits]
triSetView ([rfU, (getDir rfU) + 180, 4, 1.3, 1.0] call rfView)
triSimFrames 5
triScreenshot "eaf_pilot_wounded_back_004m"
triClearView
deleteVehicle rfU
triSimFrames 5

// --- RJAF pilot (lobo_jpilot.p3d; RJAF back/legsbk wound pairs repointed) ---
call {rfU = objNull; "LoBo_Jordan_Pilot" createUnit [[8000, 9800, 0], group r4East, "rfU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rfU, "LoBo_Jordan_Pilot"]
[rfU] call rfHold
triSimFrames 60
[rfU] call rfWound
triSimFrames 30
logInfo format ["LOBO_R4_REFS wounded=%1 damage=%2 alive=%3 hits[hlava,telo,ruce,nohy]=%4", typeOf rfU, damage rfU, alive rfU, [rfU] call rfHits]
triSetView ([rfU, (getDir rfU) + 180, 4, 1.3, 1.0] call rfView)
triSimFrames 5
triScreenshot "rjaf_pilot_wounded_back_004m"
triClearView
deleteVehicle rfU
triSimFrames 5

// --- IAF 69th Sqn pilot (lobo_iafpilot_grey.p3d; IAF\tex2 legsbk pair repointed) ---
call {rfU = objNull; "LoBo_IAF_Pilot_grey" createUnit [[8000, 9800, 0], group r4East, "rfU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rfU, "LoBo_IAF_Pilot_grey"]
[rfU] call rfHold
triSimFrames 60
[rfU] call rfWound
triSimFrames 30
logInfo format ["LOBO_R4_REFS wounded=%1 damage=%2 alive=%3 hits[hlava,telo,ruce,nohy]=%4", typeOf rfU, damage rfU, alive rfU, [rfU] call rfHits]
triSetView ([rfU, (getDir rfU) + 180, 4, 1.3, 1.0] call rfView)
triSimFrames 5
triScreenshot "iaf69_pilot_wounded_back_004m"
triClearView
deleteVehicle rfU
triSimFrames 5

// --- MiG-21bis MR1 (LoBoMig soundLandCrash repointed from soundss to sounds) ---
// The crash sounds load with the type; only that load is checked, not playback.
rfV = "LoBo_E_mig21bis_MR1" createVehicle [8000, 9800, 0]
triAssertEq [typeOf rfV, "LoBo_E_mig21bis_MR1"]
call {rfV setDir 0; rfV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R4_REFS spawned=%1 pos=%2 alive=%3", typeOf rfV, getPosASL rfV, alive rfV]
triSetView ([rfV, 270, 22, 2.5, 0.5] call rfView)
triSimFrames 5
triScreenshot "mig21bis_mr1_profile"
triClearView
deleteVehicle rfV
triSimFrames 5

// --- F-16 IAF AA (AIM-9 proxy lobo_aim9.p3d: Geometry LOD repointed) ---
// The first view repeats repair3/lobo_tail_numbers "f16iaf_a_nose" for comparison.
rfV = "Lobo_F16_IAF_AA" createVehicle [8000, 9800, 0]
triAssertEq [typeOf rfV, "Lobo_F16_IAF_AA"]
call {rfV setDir 0; rfV setPos [8000, 9800, 0]}
triSimFrames 60
rfP = getPosASL rfV
logInfo format ["LOBO_R4_REFS spawned=%1 pos=%2 alive=%3", typeOf rfV, rfP, alive rfV]
triSetView [(rfP select 0) - 7, (rfP select 2) + 1.5, (rfP select 1) + 4, 7, -0.6, 0]
triSimFrames 5
triScreenshot "f16iaf_nose_as_numbers01"
triSetView [(rfP select 0) - 9, (rfP select 2) + 0.8, (rfP select 1) - 1.5, 1, -0.05, 0.1]
triSimFrames 5
triScreenshot "f16iaf_left_rails"
triSetView [(rfP select 0) + 5, (rfP select 2) + 0.6, (rfP select 1) + 7, -0.55, -0.08, -1]
triSimFrames 5
triScreenshot "f16iaf_right_rails_front"
triClearView
deleteVehicle rfV
triSimFrames 5

// --- F-16 EAF AA (one backfuse.pac face per variant stays known missing) ---
rfV = "Lobo_F16_EAF_AA" createVehicle [8000, 9800, 0]
triAssertEq [typeOf rfV, "Lobo_F16_EAF_AA"]
call {rfV setDir 0; rfV setPos [8000, 9800, 0]}
triSimFrames 60
rfP = getPosASL rfV
logInfo format ["LOBO_R4_REFS spawned=%1 pos=%2 alive=%3", typeOf rfV, rfP, alive rfV]
triSetView [(rfP select 0) - 7, (rfP select 2) + 1.5, (rfP select 1) + 4, 7, -0.6, 0]
triSimFrames 5
triScreenshot "f16eaf_nose_as_numbers01"
triClearView
deleteVehicle rfV
triSimFrames 5
logInfo "LOBO_R4_REFS_DONE"
