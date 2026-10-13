// ============================================================================
//  R7: restored and repointed LoBo content (repair 2026-10-11.4, third and
//  fourth parts), installed @LoBo only. UD engine.
//
//  Restored members (third part), same stored names as the data asks for:
//    - LoBo_airammo.pbo: LoBo_Mk81.p3d, LoBo_Mk81/82/83/84_LDGB(_d).p3d
//      (CfgAmmo model and proxyShape of LoBo_MK81 and LoBo_MK8x_LDGB)
//    - LoBoEgypt.pbo: inf\helm_s_in.pac (LOD 10 of loboegyhi, loboegygrd,
//      loboegypara and loboegyamphi.p3d)
//  Repoints (fourth part):
//    - loBo_Snunit picture        \LoBo_FAB\Snunit.paa      -> \LoBo_FAB\SN\Snunit.paa
//    - BAS_Satcom uiPicture/picture icon\BASirto, icon\BAS_satcom -> icons\
//    - LoBo_B_Medic_MDA icon      \LoBo_Ob\bar\b4.jpg       -> \Baracken\b4.jpg
//    - M1 Guard _r LOD 15/20      tex1\kolo.pac, main_side.paa -> kolo.paa, main_side.pac
//    - F-4 Wild Weasel LOD 5      idf\mer.pac, idf\dark_grey.pac -> idf\1\mer.pac, com\dark_grey.pac
//    - F-5 (3 models) LOD 500/1100 LoBoF5\com\white.paa    -> LoBo_Kfir\kfir1\white.paa
//
//  A shape loads every LOD's textures when it loads (earlier runs log the
//  LOD 10 helmet failure right after the shape's own LOD warnings), and a
//  vehicle type loads its picture and icon (and a weapon its uiPicture) when
//  the type initialises; the gear picture of a weapon loads only when the
//  notepad's Equipment page shows it. Every object is created from the
//  script after the boot; the mission places only stock units. The pass
//  criteria are in the log (repair4/check_logs.py r7); the screenshots need
//  visual review. The engine does not report which LOD it drew, and its LOD
//  scale adapts to the frame time (Scene::LevelFromDistance2, _lodInvWidth
//  between visualQuality and 16x it). The ladders bracket both ends at
//  1280x720: men LOD 10 at about 42-62 m (coarsest) or 680-1020 m (finest),
//  M1 LOD 15/20 at about 62-83/83+ m or 1020-1360/1360+ m.
//
//  rgView [object, bearing, distance, camera height, aim height]: camera on the
//  compass bearing from the object (0 north, 90 east), looking back at it,
//  heights above the ground under the object.
// ============================================================================
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
setViewDistance 1500
showCinemaBorder false
player allowDammage false
player setCaptive true
call {player setPos [7800, 9450, 0]}
rgView = {_o = _this select 0; _b = _this select 1; _d = _this select 2; _h = _this select 3; _t = _this select 4; _p = getPosASL _o; _g = (_p select 2) - ((getPos _o) select 2); [(_p select 0) + _d * (sin _b), _g + _h, (_p select 1) + _d * (cos _b), 0 - (sin _b), (_t - _h) / _d, 0 - (cos _b)]}
rgHold = {_u = _this select 0; _u setCaptive true; _u disableAI "MOVE"; _u disableAI "TARGET"; _u disableAI "AUTOTARGET"; _u stop true; _u setDir (_this select 1)}
rgShotView = {_p = getPosASL (_this select 0); [(_p select 0) - 4, (_p select 2) + 0.3, (_p select 1), 1, -0.07, 0]}
logInfo format ["LOBO_R7_START lodBias=%1 fov=%2", triGetObjectLodBias, triCamFov]

// ---------------------------------------------------------------------------
// A1. The restored bomb models as shots. createVehicle of a CfgAmmo class makes
//     a shot (NewShot), whose type loads its model and proxyShape. Created
//     250 m above open ground east of the runway, photographed from 4 m west.
// ---------------------------------------------------------------------------
rgB = "LoBo_MK81" createVehicle [8300, 9800, 250]
logInfo format ["LOBO_R7 ammo class=LoBo_MK81 typeOf=%1 ok=%2 posASL=%3 model=LoBo_Mk81 proxyShape=LoBo_Mk81", typeOf rgB, (typeOf rgB) == "LoBo_MK81", getPosASL rgB]
triSetView ([rgB] call rgShotView)
triSimFrames 1
triScreenshot "ammo_mk81_shot"
call {if (!isNull rgB) then {deleteVehicle rgB}}
triSimFrames 2
rgB = "LoBo_MK81_LDGB" createVehicle [8300, 9800, 250]
logInfo format ["LOBO_R7 ammo class=LoBo_MK81_LDGB typeOf=%1 ok=%2 posASL=%3 model=LoBo_Mk81_LDGB_d proxyShape=LoBo_Mk81_LDGB", typeOf rgB, (typeOf rgB) == "LoBo_MK81_LDGB", getPosASL rgB]
triSetView ([rgB] call rgShotView)
triSimFrames 1
triScreenshot "ammo_mk81_ldgb_shot"
call {if (!isNull rgB) then {deleteVehicle rgB}}
triSimFrames 2
rgB = "LoBo_MK82_LDGB" createVehicle [8300, 9800, 250]
logInfo format ["LOBO_R7 ammo class=LoBo_MK82_LDGB typeOf=%1 ok=%2 posASL=%3 model=LoBo_Mk82_LDGB_d proxyShape=LoBo_Mk82_LDGB", typeOf rgB, (typeOf rgB) == "LoBo_MK82_LDGB", getPosASL rgB]
triSetView ([rgB] call rgShotView)
triSimFrames 1
triScreenshot "ammo_mk82_ldgb_shot"
call {if (!isNull rgB) then {deleteVehicle rgB}}
triSimFrames 2
rgB = "LoBo_MK83_LDGB" createVehicle [8300, 9800, 250]
logInfo format ["LOBO_R7 ammo class=LoBo_MK83_LDGB typeOf=%1 ok=%2 posASL=%3 model=LoBo_Mk83_LDGB_D proxyShape=LoBo_Mk83_LDGB", typeOf rgB, (typeOf rgB) == "LoBo_MK83_LDGB", getPosASL rgB]
triSetView ([rgB] call rgShotView)
triSimFrames 1
triScreenshot "ammo_mk83_ldgb_shot"
call {if (!isNull rgB) then {deleteVehicle rgB}}
triSimFrames 2
rgB = "LoBo_MK84_LDGB" createVehicle [8300, 9800, 250]
logInfo format ["LOBO_R7 ammo class=LoBo_MK84_LDGB typeOf=%1 ok=%2 posASL=%3 model=LoBo_Mk84_LDGB_D proxyShape=LoBo_Mk84_LDGB", typeOf rgB, (typeOf rgB) == "LoBo_MK84_LDGB", getPosASL rgB]
triSetView ([rgB] call rgShotView)
triSimFrames 1
triScreenshot "ammo_mk84_ldgb_shot"
call {if (!isNull rgB) then {deleteVehicle rgB}}
triSimFrames 2
triClearView

// ---------------------------------------------------------------------------
// A2. Aircraft that carry the bombs on their pylons (the magazine's ammo type
//     draws its proxyShape there). Each faces north on the runway line x=8000.
// ---------------------------------------------------------------------------
rgV = "LoBo_E_mig21bis_MR1" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 carrier class=LoBo_E_mig21bis_MR1 typeOf=%1 ok=%2 posASL=%3 bombs=%4 ammo=LoBo_MK81", typeOf rgV, (typeOf rgV) == "LoBo_E_mig21bis_MR1", getPosASL rgV, rgV ammo "LoBo_Mk81Rail2"]
triAssertEq [(typeOf rgV) == "LoBo_E_mig21bis_MR1", true]
triSetView ([rgV, 270, 11, 0.9, 0.8] call rgView)
triSimFrames 5
triScreenshot "mig21mr1_left_low"
triSetView ([rgV, 315, 9, 0.6, 0.9] call rgView)
triSimFrames 5
triScreenshot "mig21mr1_front_left_low"
triClearView
deleteVehicle rgV
triSimFrames 5
rgV = "LoBo_E_mig21bis_MR2" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 carrier class=LoBo_E_mig21bis_MR2 typeOf=%1 ok=%2 posASL=%3 bombs=%4 ammo=LoBo_MK81", typeOf rgV, (typeOf rgV) == "LoBo_E_mig21bis_MR2", getPosASL rgV, rgV ammo "LoBo_Mk81Rail2"]
triAssertEq [(typeOf rgV) == "LoBo_E_mig21bis_MR2", true]
triSetView ([rgV, 270, 11, 0.9, 0.8] call rgView)
triSimFrames 5
triScreenshot "mig21mr2_left_low"
triClearView
deleteVehicle rgV
triSimFrames 5
rgV = "LoBo_A4" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 carrier class=LoBo_A4 typeOf=%1 ok=%2 posASL=%3 bombs=%4 ammo=LoBo_MK81", typeOf rgV, (typeOf rgV) == "LoBo_A4", getPosASL rgV, rgV ammo "LoBo_Mk81Rail12"]
triAssertEq [(typeOf rgV) == "LoBo_A4", true]
triSetView ([rgV, 270, 11, 0.9, 0.8] call rgView)
triSimFrames 5
triScreenshot "a4_left_low"
triSetView ([rgV, 315, 9, 0.6, 0.9] call rgView)
triSimFrames 5
triScreenshot "a4_front_left_low"
triClearView
deleteVehicle rgV
triSimFrames 5
rgV = "LoBo_F15I_IAF_grey_MK84" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 carrier class=LoBo_F15I_IAF_grey_MK84 typeOf=%1 ok=%2 posASL=%3 mk84=%4 mk82=%5 ammo=LoBo_MK84_LDGB,LoBo_MK82_LDGB", typeOf rgV, (typeOf rgV) == "LoBo_F15I_IAF_grey_MK84", getPosASL rgV, rgV ammo "LoBo_Mk84LDGBRail6", rgV ammo "LoBo_Mk82LDGBRail6"]
triAssertEq [(typeOf rgV) == "LoBo_F15I_IAF_grey_MK84", true]
triSetView ([rgV, 270, 13, 0.9, 0.8] call rgView)
triSimFrames 5
triScreenshot "f15i_mk84_left_low"
triSetView ([rgV, 315, 10, 0.6, 0.9] call rgView)
triSimFrames 5
triScreenshot "f15i_mk84_front_left_low"
triClearView
deleteVehicle rgV
triSimFrames 5
rgV = "LoBo_F16I_IAF_B" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 carrier class=LoBo_F16I_IAF_B typeOf=%1 ok=%2 posASL=%3 mk82=%4 ammo=LoBo_MK82_LDGB", typeOf rgV, (typeOf rgV) == "LoBo_F16I_IAF_B", getPosASL rgV, rgV ammo "LoBo_Mk82LDGBRail12"]
triAssertEq [(typeOf rgV) == "LoBo_F16I_IAF_B", true]
triSetView ([rgV, 270, 12, 0.9, 0.8] call rgView)
triSimFrames 5
triScreenshot "f16i_b_left_low"
triSetView ([rgV, 315, 9, 0.6, 0.9] call rgView)
triSimFrames 5
triScreenshot "f16i_b_front_left_low"
triClearView
deleteVehicle rgV
triSimFrames 5

// ---------------------------------------------------------------------------
// A3. Drop from altitude: an A-4 with an AI pilot, 400 m up and flying south
//     at 110 m/s well south of the runway objects, fires one Mk-81 from
//     LoBo_Mk81Rail12 (the weapon -> shot path). Best effort, no assert.
// ---------------------------------------------------------------------------
rgA = "LoBo_A4" createVehicle [8000, 7600, 0]
call {rgPil = objNull; "SoldierEB" createUnit [[8015, 7600, 0], group r4East, "rgPil = this", 0.6, "PRIVATE"]}
triAssertEq [(typeOf rgA) == "LoBo_A4", true]
triAssertEq [(typeOf rgPil) == "SoldierEB", true]
call {rgPil moveInDriver rgA; rgA engineOn true; rgA flyInHeight 400}
call {rgA setDir 180; rgA setPos [8000, 7600, 400]; rgA setVelocity [0, -110, 0]}
triSimFrames 20
rgN0 = rgA ammo "LoBo_Mk81Rail12"
rgA fire "LoBo_Mk81Rail12"
triSimFrames 2
rgP = getPosASL rgA
triSetView [(rgP select 0) + 30, (rgP select 2) - 6, (rgP select 1) - 4, -1, 0.15, 0.1]
triSimFrames 1
triScreenshot "a4_mk81_release_in_flight"
triSimFrames 30
logInfo format ["LOBO_R7 drop carrier=LoBo_A4 weapon=LoBo_Mk81Rail12 ammoBefore=%1 ammoAfter=%2 altAGL=%3 alive=%4 pilotIn=%5", rgN0, rgA ammo "LoBo_Mk81Rail12", (getPos rgA) select 2, alive rgA, (driver rgA) == rgPil]
triClearView
call {{deleteVehicle _x} forEach crew rgA; deleteVehicle rgA}
triSimFrames 5

// ---------------------------------------------------------------------------
// B. Egyptian infantry (restored LoBoEgypt\inf\helm_s_in.pac, LOD 10). Each
//    faces south at [8000, 9800]; camera south of him, distance ladder.
// ---------------------------------------------------------------------------
call {rgU = objNull; "LoBo_Egypt_infantry" createUnit [[8000, 9800, 0], group r4East, "rgU = this", 0.6, "PRIVATE"]}
[rgU, 180] call rgHold
triSimFrames 60
logInfo format ["LOBO_R7 egy class=LoBo_Egypt_infantry typeOf=%1 ok=%2 posASL=%3 dir=%4 model=LoBoEgyHI fps=%5", typeOf rgU, (typeOf rgU) == "LoBo_Egypt_infantry", getPosASL rgU, getDir rgU, triFps]
triAssertEq [(typeOf rgU) == "LoBo_Egypt_infantry", true]
triSetView ([rgU, 180, 4, 1.6, 1.4] call rgView)
triSimFrames 5
triScreenshot "egyhi_front_004m"
triSetView ([rgU, 180, 50, 2.2, 1.0] call rgView)
triSimFrames 5
triScreenshot "egyhi_front_050m"
triSetView ([rgU, 180, 300, 6, 1.0] call rgView)
triSimFrames 5
triScreenshot "egyhi_front_300m"
triSetView ([rgU, 180, 850, 16, 1.0] call rgView)
triSimFrames 5
triScreenshot "egyhi_front_850m"
triClearView
deleteVehicle rgU
triSimFrames 5
call {rgU = objNull; "LoBo_Egypt_Grd" createUnit [[8000, 9800, 0], group r4East, "rgU = this", 0.6, "PRIVATE"]}
[rgU, 180] call rgHold
triSimFrames 60
logInfo format ["LOBO_R7 egy class=LoBo_Egypt_Grd typeOf=%1 ok=%2 posASL=%3 dir=%4 model=LoBoEgyGrd fps=%5", typeOf rgU, (typeOf rgU) == "LoBo_Egypt_Grd", getPosASL rgU, getDir rgU, triFps]
triAssertEq [(typeOf rgU) == "LoBo_Egypt_Grd", true]
triSetView ([rgU, 180, 4, 1.6, 1.4] call rgView)
triSimFrames 5
triScreenshot "egygrd_front_004m"
triSetView ([rgU, 180, 50, 2.2, 1.0] call rgView)
triSimFrames 5
triScreenshot "egygrd_front_050m"
triSetView ([rgU, 180, 300, 6, 1.0] call rgView)
triSimFrames 5
triScreenshot "egygrd_front_300m"
triSetView ([rgU, 180, 850, 16, 1.0] call rgView)
triSimFrames 5
triScreenshot "egygrd_front_850m"
triClearView
deleteVehicle rgU
triSimFrames 5
call {rgU = objNull; "LoBo_Egypt_Para" createUnit [[8000, 9800, 0], group r4East, "rgU = this", 0.6, "PRIVATE"]}
[rgU, 180] call rgHold
triSimFrames 60
logInfo format ["LOBO_R7 egy class=LoBo_Egypt_Para typeOf=%1 ok=%2 posASL=%3 dir=%4 model=LoBoEgypara fps=%5", typeOf rgU, (typeOf rgU) == "LoBo_Egypt_Para", getPosASL rgU, getDir rgU, triFps]
triAssertEq [(typeOf rgU) == "LoBo_Egypt_Para", true]
triSetView ([rgU, 180, 4, 1.6, 1.4] call rgView)
triSimFrames 5
triScreenshot "egypara_front_004m"
triSetView ([rgU, 180, 50, 2.2, 1.0] call rgView)
triSimFrames 5
triScreenshot "egypara_front_050m"
triSetView ([rgU, 180, 300, 6, 1.0] call rgView)
triSimFrames 5
triScreenshot "egypara_front_300m"
triSetView ([rgU, 180, 850, 16, 1.0] call rgView)
triSimFrames 5
triScreenshot "egypara_front_850m"
triClearView
deleteVehicle rgU
triSimFrames 5
call {rgU = objNull; "LoBo_Egypt_AmpInfantry" createUnit [[8000, 9800, 0], group r4East, "rgU = this", 0.6, "PRIVATE"]}
[rgU, 180] call rgHold
triSimFrames 60
logInfo format ["LOBO_R7 egy class=LoBo_Egypt_AmpInfantry typeOf=%1 ok=%2 posASL=%3 dir=%4 model=LoBoEgyAmpHI fps=%5", typeOf rgU, (typeOf rgU) == "LoBo_Egypt_AmpInfantry", getPosASL rgU, getDir rgU, triFps]
triAssertEq [(typeOf rgU) == "LoBo_Egypt_AmpInfantry", true]
triSetView ([rgU, 180, 4, 1.6, 1.4] call rgView)
triSimFrames 5
triScreenshot "egyamphi_front_004m"
triSetView ([rgU, 180, 50, 2.2, 1.0] call rgView)
triSimFrames 5
triScreenshot "egyamphi_front_050m"
triSetView ([rgU, 180, 300, 6, 1.0] call rgView)
triSimFrames 5
triScreenshot "egyamphi_front_300m"
triSetView ([rgU, 180, 850, 16, 1.0] call rgView)
triSimFrames 5
triScreenshot "egyamphi_front_850m"
triClearView
deleteVehicle rgU
triSimFrames 5

// ---------------------------------------------------------------------------
// C. Egyptian Guard M1A1/M1A2 (_r models, LOD 15 and 20 repointed). Facing
//    east, camera south of the tank, distance ladder.
// ---------------------------------------------------------------------------
rgV = "LoBo_M1G" createVehicle [8000, 9800, 0]
call {rgV setDir 90; rgV setPos [8000, 9800, 0]}
triSimFrames 30
logInfo format ["LOBO_R7 m1 class=LoBo_M1G typeOf=%1 ok=%2 posASL=%3 model=LoBo_M1A1E_R fps=%4", typeOf rgV, (typeOf rgV) == "LoBo_M1G", getPosASL rgV, triFps]
triAssertEq [(typeOf rgV) == "LoBo_M1G", true]
triSetView ([rgV, 180, 12, 2.0, 1.3] call rgView)
triSimFrames 5
triScreenshot "m1g_side_012m"
triSetView ([rgV, 180, 75, 3, 1.5] call rgView)
triSimFrames 5
triScreenshot "m1g_side_075m"
triSetView ([rgV, 180, 300, 8, 1.5] call rgView)
triSimFrames 5
triScreenshot "m1g_side_300m"
triSetView ([rgV, 180, 1200, 32, 1.5] call rgView)
triSimFrames 5
triScreenshot "m1g_side_1200m"
triSetView ([rgV, 180, 1420, 40, 1.5] call rgView)
triSimFrames 5
triScreenshot "m1g_side_1420m"
triClearView
deleteVehicle rgV
triSimFrames 5
rgV = "LoBo_M1GMG_2" createVehicle [8000, 9800, 0]
call {rgV setDir 90; rgV setPos [8000, 9800, 0]}
triSimFrames 30
logInfo format ["LOBO_R7 m1 class=LoBo_M1GMG_2 typeOf=%1 ok=%2 posASL=%3 model=LoBo_M1A2E_R fps=%4", typeOf rgV, (typeOf rgV) == "LoBo_M1GMG_2", getPosASL rgV, triFps]
triAssertEq [(typeOf rgV) == "LoBo_M1GMG_2", true]
triSetView ([rgV, 180, 12, 2.0, 1.3] call rgView)
triSimFrames 5
triScreenshot "m1gmg2_side_012m"
triSetView ([rgV, 180, 75, 3, 1.5] call rgView)
triSimFrames 5
triScreenshot "m1gmg2_side_075m"
triSetView ([rgV, 180, 300, 8, 1.5] call rgView)
triSimFrames 5
triScreenshot "m1gmg2_side_300m"
triSetView ([rgV, 180, 1200, 32, 1.5] call rgView)
triSimFrames 5
triScreenshot "m1gmg2_side_1200m"
triSetView ([rgV, 180, 1420, 40, 1.5] call rgView)
triSimFrames 5
triScreenshot "m1gmg2_side_1420m"
triClearView
deleteVehicle rgV
triSimFrames 5

// ---------------------------------------------------------------------------
// D. F-4E Kurnas Wild Weasel (LOD 5, its first LOD: ECM pod faces repointed).
// ---------------------------------------------------------------------------
rgV = "LoBo_F4_WW_IAF" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 f4 class=LoBo_F4_WW_IAF typeOf=%1 ok=%2 posASL=%3 model=LoBo_F4_WW_IAF", typeOf rgV, (typeOf rgV) == "LoBo_F4_WW_IAF", getPosASL rgV]
triAssertEq [(typeOf rgV) == "LoBo_F4_WW_IAF", true]
triSetView ([rgV, 270, 14, 1.6, 1.2] call rgView)
triSimFrames 5
triScreenshot "f4ww_left"
triSetView ([rgV, 315, 9, 0.45, 1.0] call rgView)
triSimFrames 5
triScreenshot "f4ww_front_left_under"
triSetView ([rgV, 225, 10, 0.45, 1.0] call rgView)
triSimFrames 5
triScreenshot "f4ww_rear_left_under"
triSetView ([rgV, 90, 14, 1.6, 1.2] call rgView)
triSimFrames 5
triScreenshot "f4ww_right"
triClearView
deleteVehicle rgV
triSimFrames 5

// ---------------------------------------------------------------------------
// E. RJAF F-5E, three models (white.paa in LODs 500 and 1100, the pilot view).
//    Profile from outside, then the player in the cockpit with the internal
//    camera, then out again. switchCamera goes to the plane, not the player:
//    on the seated person the camera stays at his last on-foot position
//    (restored-01 rendered the grass at [7800, 9450] with no cockpit).
// ---------------------------------------------------------------------------
rgV = "LoBo_F5_AA_Jor" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 f5 class=LoBo_F5_AA_Jor typeOf=%1 ok=%2 posASL=%3 model=LoBo_F5_AA_Jor", typeOf rgV, (typeOf rgV) == "LoBo_F5_AA_Jor", getPosASL rgV]
triAssertEq [(typeOf rgV) == "LoBo_F5_AA_Jor", true]
triSetView ([rgV, 270, 12, 1.8, 1.2] call rgView)
triSimFrames 5
triScreenshot "f5aa_profile"
triClearView
call {player moveInDriver rgV; rgV switchCamera "INTERNAL"}
triSimFrames 60
logInfo format ["LOBO_R7 f5cockpit class=LoBo_F5_AA_Jor inDriver=%1 camera=%2 camPos=%3 planeASL=%4", (driver rgV) == player, triCamView, triCamPos, getPosASL rgV]
triScreenshot "f5aa_pilot_view"
call {player action ["GETOUT", rgV]}
triSimFrames 120
call {if (vehicle player != player) then {player action ["EJECT", rgV]}}
triSimFrames 30
logInfo format ["LOBO_R7 f5exit class=LoBo_F5_AA_Jor out=%1", (vehicle player) == player]
call {player switchCamera "EXTERNAL"; player setPos [7800, 9450, 0]}
deleteVehicle rgV
triSimFrames 5
rgV = "LoBo_F5_mir_Jor" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 f5 class=LoBo_F5_mir_Jor typeOf=%1 ok=%2 posASL=%3 model=LoBo_F5_MIR_Jor", typeOf rgV, (typeOf rgV) == "LoBo_F5_mir_Jor", getPosASL rgV]
triAssertEq [(typeOf rgV) == "LoBo_F5_mir_Jor", true]
triSetView ([rgV, 270, 12, 1.8, 1.2] call rgView)
triSimFrames 5
triScreenshot "f5mir_profile"
triClearView
call {player moveInDriver rgV; rgV switchCamera "INTERNAL"}
triSimFrames 60
logInfo format ["LOBO_R7 f5cockpit class=LoBo_F5_mir_Jor inDriver=%1 camera=%2 camPos=%3 planeASL=%4", (driver rgV) == player, triCamView, triCamPos, getPosASL rgV]
triScreenshot "f5mir_pilot_view"
call {player action ["GETOUT", rgV]}
triSimFrames 120
call {if (vehicle player != player) then {player action ["EJECT", rgV]}}
triSimFrames 30
logInfo format ["LOBO_R7 f5exit class=LoBo_F5_mir_Jor out=%1", (vehicle player) == player]
call {player switchCamera "EXTERNAL"; player setPos [7800, 9450, 0]}
deleteVehicle rgV
triSimFrames 5
rgV = "LoBo_F5_AT_Jor" createVehicle [8000, 9800, 0]
call {rgV setDir 0; rgV setPos [8000, 9800, 0]}
triSimFrames 60
logInfo format ["LOBO_R7 f5 class=LoBo_F5_AT_Jor typeOf=%1 ok=%2 posASL=%3 model=LoBo_F5_AT_Jor", typeOf rgV, (typeOf rgV) == "LoBo_F5_AT_Jor", getPosASL rgV]
triAssertEq [(typeOf rgV) == "LoBo_F5_AT_Jor", true]
triSetView ([rgV, 270, 12, 1.8, 1.2] call rgView)
triSimFrames 5
triScreenshot "f5at_profile"
triClearView
call {player moveInDriver rgV; rgV switchCamera "INTERNAL"}
triSimFrames 60
logInfo format ["LOBO_R7 f5cockpit class=LoBo_F5_AT_Jor inDriver=%1 camera=%2 camPos=%3 planeASL=%4", (driver rgV) == player, triCamView, triCamPos, getPosASL rgV]
triScreenshot "f5at_pilot_view"
call {player action ["GETOUT", rgV]}
triSimFrames 120
call {if (vehicle player != player) then {player action ["EJECT", rgV]}}
triSimFrames 30
logInfo format ["LOBO_R7 f5exit class=LoBo_F5_AT_Jor out=%1", (vehicle player) == player]
call {player switchCamera "EXTERNAL"; player setPos [7800, 9450, 0]}
deleteVehicle rgV
triSimFrames 5

// ---------------------------------------------------------------------------
// F1. Medic barrack (LoBo_Ob icon repointed to \Baracken\b4.jpg; the icon is
//     the type's map/editor icon and loads with the type).
// ---------------------------------------------------------------------------
rgV = "LoBo_B_Medic_MDA" createVehicle [8000, 9800, 0]
triSimFrames 30
logInfo format ["LOBO_R7 medic class=LoBo_B_Medic_MDA typeOf=%1 ok=%2 posASL=%3 icon=Baracken-b4.jpg", typeOf rgV, (typeOf rgV) == "LoBo_B_Medic_MDA", getPosASL rgV]
triAssertEq [(typeOf rgV) == "LoBo_B_Medic_MDA", true]
triSetView ([rgV, 180, 25, 3, 2.5] call rgView)
triSimFrames 5
triScreenshot "medic_mda_building"
triClearView
deleteVehicle rgV
triSimFrames 5

// ---------------------------------------------------------------------------
// F2. Group bar: the player drives the Snunit (bar shows the vehicle picture,
//     repointed \LoBo_FAB\SN\Snunit.paa) and a group member on foot carries
//     BAS_Satcom as his secondary weapon (bar shows its uiPicture, repointed
//     \LoBoRadios\icons\BASirto.paa).
// ---------------------------------------------------------------------------
rgV = "loBo_Snunit" createVehicle [8000, 9800, 0]
call {rgV setDir 90; rgV setPos [8000, 9800, 0]}
triSimFrames 30
logInfo format ["LOBO_R7 snunit class=loBo_Snunit typeOf=%1 ok=%2 posASL=%3 picture=LoBo_FAB-SN-Snunit.paa", typeOf rgV, (typeOf rgV) == "loBo_Snunit", getPosASL rgV]
triAssertEq [(typeOf rgV) == "loBo_Snunit", true]
triSetView ([rgV, 180, 14, 2.0, 0.8] call rgView)
triSimFrames 5
triScreenshot "snunit_side_014m"
triClearView
call {rgMate = objNull; "SoldierWB" createUnit [[7805, 9450, 0], group player, "rgMate = this", 0.6, "PRIVATE"]}
triAssertEq [(typeOf rgMate) == "SoldierWB", true]
call {rgMate addWeapon "BAS_Satcom"; rgMate disableAI "MOVE"; rgMate stop true}
call {player moveInDriver rgV; rgV switchCamera "EXTERNAL"}
triSimFrames 60
logInfo format ["LOBO_R7 groupbar boat=%1 playerDriver=%2 mateWeapons=%3 units=%4", typeOf rgV, (driver rgV) == player, weapons rgMate, count units group player]
triScreenshot "groupbar_snunit_picture_and_basirto"

// ---------------------------------------------------------------------------
// F3. Gear: the notepad's Equipment page draws each weapon's picture; BAS_Satcom
//     names \LoBoRadios\icons\BAS_satcom.paa (repointed from icon\).
// ---------------------------------------------------------------------------
call {player action ["GETOUT", rgV]}
triSimFrames 120
call {player switchCamera "EXTERNAL"; player addWeapon "BAS_Satcom"}
logInfo format ["LOBO_R7 gearprep out=%1 playerWeapons=%2", (vehicle player) == player, weapons player]
triOpenMap
triSendKey 16
triSimFrames 10
rgSec = triBriefingSwitch "Equipment"
triSimFrames 10
logInfo format ["LOBO_R7 gear section=%1", rgSec]
triScreenshot "gear_equipment_bas_satcom"

logInfo "LOBO_R7_DONE"
triSimFrames 30
triEndTest
