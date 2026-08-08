// ============================================================================
//  @LoBo asset showcase - Southern Sinai.  NOT a regression test: this is a
//  screenshot generator driven through the Trident harness, because the
//  harness is the only thing that can boot the game, stage a scene
//  deterministically and capture a frame without a human at the keyboard.
//
//  Output: one PNG + BMP per triScreenshot into --output-dir.
//  See tools/screenshots/README.md for the run command and the asset roster.
//
//  HOW THE CAMERA WORKS HERE (all four learned the hard way):
//
//   1. Framing is triSetView [east, UP, north, dirE, dirUp, dirN] - absolute
//      world coordinates, engine axis order (Y is elevation).  The cutscene
//      camera (camCreate/camSetRelPos) was tried first and aimed at the sky;
//      camSetDir could not rescue it at the time because it and camSetBank
//      were both mis-wired to CamSetDive in the command table
//      (GameStateExt.cpp ~1374).  That is fixed now and camSetDir does turn
//      the camera, but triSetView stays the framing primitive here: it takes
//      the aim point straight from getPosASL arithmetic in one call, with no
//      commit and no angle conversion.
//   2. Every coordinate is computed in-script from `getPosASL obj`, which
//      returns [east, north, ASL], so no terrain height is ever hardcoded -
//      the same scene block works on a 7 m quay or a 280 m hilltop.
//   3. A cutscene camera IS still created, purely for its side effect: the
//      gameplay HUD (ammo counter, action menu, crosshair) is gated on
//      `!GWorld->GetCameraEffect()` (DisplayUIMenus.cpp:974), so without an
//      active camera effect every frame is stamped with UI.  Its own framing
//      is irrelevant - triSetView overrides it.
//   4. triScreenshot must be given a STRING LITERAL.  The runner parses the
//      literal out of the statement to maintain its own sequence counter, so a
//      `format [...]` label desyncs it and the run dies on
//      FAIL:screenshot_not_written.
//
//  WHERE THE SETS ARE.  The Guerrilla.Sinai zone positions are useless as
//  camera locations - "El Tor" at {6827,2808} is bare dune, and the authored
//  elevations are wrong (it claims 160 m at the Camp; the ground is 64.7 m).
//  These anchors instead come from clustering the 34k objects in the world
//  file offline:
//      PoseidonTools terrain objects sinai.wrp
//  whose columns are [east, ELEVATION, north] - not the order the header
//  suggests.  Each anchor is a real cluster:
//      Israeli Fire Base   [ 9330, 11310] el 186  barracks, towers, revetments
//      Egyptian Fire Base  [ 6900,  9110] el 277  ridge-top base on a road
//      Refugee Camp road   [11900,  9808] el  17  road south of the shanties,
//                                                 mosque and minaret behind
//      Eilat               [10250, 10120] el   7  container port + high-rises
//      As-Suways           [ 1440, 11030] el  10  apartments, shopfronts,
//                                                 a church - the best street
//      Egyptian airbase    [ 3470,  1600] el  40  apron, hangars, tower
//      Helipads            [ 9096,  2840] el  20  hangar and two pads
//      St Katherine        [ 6516,  6768] el 215  palm oasis + monastery gate
//
//  FRAMING RULES that came out of the earlier passes:
//   * Group of vehicles: gap 6.5 m, camera ~13 m out and 3.4 m up.  Wider
//     spacing plus a longer lens made everything tiny in a sea of sand.
//   * Aircraft: gap 17 m, camera ~17 m out and 4.5 m up.
//   * Infantry line: gap 1.9 m, camera ~9 m out and 2 m up.
//   * Anything staged inside a built-up area needs the camera above roof
//     height or well outside the walls - earlier passes shot the inside of a
//     hangar, a shanty roof and the monastery wall.
//
//  OTHER HARNESS NOTES:
//   * Trident splits this file on newlines/semicolons at brace depth 0 and
//     sends each statement as a SEPARATE eval, so _locals do NOT survive from
//     one line to the next.  A multi-line `{ ... }` block is sent as ONE eval.
//     Everything shared between statements is a global (ss*).
//   * createGroup returns grpNull for a side with no center, and the units
//     then silently never appear.  Sinai has no RESISTANCE or CIVILIAN center,
//     so the shoot calls createCenter first (it is idempotent).
//   * Clock: sunset is around 16:30 on this world.  15:00 is clean daylight,
//     15:40 is golden hour, 17:30 is already night.
//   * Everything spawned is setCaptive true, so no set turns into a firefight.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

setViewDistance 3500
0 setOvercast 0.05
0 setFog 0.02
showCinemaBorder false

player setPos [2000, 6000, 0]
player setCaptive true

ssCam = "camera" camCreate [7900, 3200, 30]
ssCam cameraEffect ["internal", "back"]
triSimFrames 20

createCenter west
createCenter east
createCenter resistance
createCenter civilian

ssAim = {
    ssT = getPosASL ssTgt;
    ssCx = (ssT select 0) + (ssOff select 0);
    ssCy = (ssT select 2) + (ssOff select 1);
    ssCz = (ssT select 1) + (ssOff select 2);
    triSetView [ssCx, ssCy, ssCz, (ssT select 0) - ssCx, ((ssT select 2) + ssAimH) - ssCy, (ssT select 1) - ssCz]
}

ssPose = {
    { _x setCaptive true; _x setBehaviour "COMBAT"; _x setUnitPos "UP"; _x setDir ssFace; _x disableAI "MOVE" } forEach (units ssGrp)
}

ssLine = {
    ssI = 0;
    while "ssI < count ssMen" do {
        (ssMen select ssI) createUnit [[ssLx + ssI * ssGap, ssLz - ssI * 0.5, 0], ssGrp, "", 0.4, "PRIVATE"];
        ssI = ssI + 1;
    }
}

ssPark = {
    ssJunk = [];
    ssI = 0;
    while "ssI < count ssCls" do {
        ssV = (ssCls select ssI) createVehicle [ssLx + ssI * ssGap, ssLz + ssI * ssGap2, 0];
        ssV setDir ssVDir;
        ssV setCaptive true;
        ssJunk = ssJunk + [ssV];
        ssI = ssI + 1;
    };
    ssHero = ssJunk select 0
}

ssWipe = { { deleteVehicle _x } forEach ssJunk; ssJunk = [] }
ssWipeMen = { { deleteVehicle _x } forEach (units ssGrp) }

ssJunk = []
ssGrp = grpNull
ssGap2 = 0

// ============================================================================
//  Israeli Fire Base, hill 186 - IDF armour
// ============================================================================
setDate [1985, 6, 15, 15, 40]
call { ssCls = ["LoBoMerkava2", "LoBo_Magach7C", "LoBo_M60A1_IDF", "LoBo_Zelda"]; ssLx = 9330; ssLz = 11310; ssGap = 5; ssGap2 = 4.5; ssVDir = 315; call ssPark }
triSimFrames 60
ssTgt = ssHero
ssAimH = 2.0
ssOff = [-12, 3.6, -11]
call ssAim
triSimFrames 12
triScreenshot "01_idf_armour_park"
triAssertGt [(triGetPixelMaxChannel [0.5, 0.5]), 20]

ssOff = [-7, 1.5, -6.5]
ssAimH = 1.7
call ssAim
triSimFrames 8
triScreenshot "02_merkava_mk2_hero"

ssTgt = ssJunk select 1
ssOff = [-7, 1.5, -6.5]
call ssAim
triSimFrames 8
triScreenshot "03_magach_7c"

call ssWipe
triSimFrames 8

call { ssCls = ["LoBoMerkava4g", "LoBo_Achzarit", "LoBo_Centurion", "LoBo_M1"]; ssLx = 9330; ssLz = 11310; ssGap = 5; ssGap2 = 4.5; ssVDir = 315; call ssPark }
triSimFrames 60
ssTgt = ssHero
ssAimH = 2.0
ssOff = [-12, 3.6, -11]
call ssAim
triSimFrames 12
triScreenshot "04_merkava_mk4_and_achzarit"

ssOff = [-7, 1.4, -6.5]
ssAimH = 1.7
call ssAim
triSimFrames 8
triScreenshot "05_merkava_mk4_hero"

ssTgt = ssJunk select 1
ssOff = [-6.5, 1.4, -6]
ssAimH = 1.5
call ssAim
triSimFrames 8
triScreenshot "06_achzarit_heavy_apc"

call ssWipe
triSimFrames 8

// Golani section with its Humvee, M35 and Sufa
call {
    ssGrp = createGroup west;
    ssCls = ["LoBo_HMWVUP_IDF_M240", "LoBo_M35", "LoBo_sufa"]; ssLx = 9318; ssLz = 11322; ssGap = -8; ssGap2 = 6; ssVDir = 250; call ssPark;
    ssMen = ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWLAW", "LoBoGolaniWBSN", "LoBoGolaniWMedic"];
    ssLx = 9324; ssLz = 11313; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 6 }
ssFace = 165
call ssPose
triSimFrames 60
ssTgt = units ssGrp select 2
ssAimH = 1.5
ssOff = [2, 1.9, -8]
call ssAim
triSimFrames 12
triScreenshot "07_golani_section"

ssTgt = units ssGrp select 1
ssOff = [1.0, 1.6, -3.4]
call ssAim
triSimFrames 8
triScreenshot "08_golani_rifleman"

ssTgt = ssHero
ssAimH = 1.5
ssOff = [5, 2.2, -8]
call ssAim
triSimFrames 8
triScreenshot "09_humvee_and_soft_skins"

call ssWipeMen
call ssWipe
triSimFrames 8

call {
    ssGrp = createGroup west;
    ssMen = ["LoBoShayetetWB", "LoBoShayetetWMG", "LoBoShayetetSN", "LoBoParaWB", "LoBoParaWMG", "LoBoParaWLAW"];
    ssLx = 9324; ssLz = 11313; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 6 }
ssFace = 165
call ssPose
triSimFrames 60
ssTgt = units ssGrp select 2
ssAimH = 1.5
ssOff = [2, 1.9, -8]
call ssAim
triSimFrames 12
triScreenshot "10_shayetet_and_paratroopers"

ssTgt = units ssGrp select 4
ssOff = [1.0, 1.6, -3.4]
call ssAim
triSimFrames 8
triScreenshot "11_paratrooper_machinegunner"

call ssWipeMen
triSimFrames 8

// ============================================================================
//  Egyptian Fire Base, ridge 277
// ============================================================================
call {
    ssGrp = createGroup east;
    ssCls = ["LoBo_T55_EGY_1", "LoBo_BMP_EGY", "LoBo_Ural_egy", "LoBo_UAZ_Egy"]; ssLx = 6900; ssLz = 9110; ssGap = -5.5; ssGap2 = 5; ssVDir = 135; call ssPark;
    ssMen = ["LoBo_Egypt_FrtCrpo", "LoBo_Egypt_FrtCrp", "LoBo_Egypt_FrtCrpMG", "LoBo_Egypt_FrtCrpAT"];
    ssLx = 6897; ssLz = 9103; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 175
call ssPose
triSimFrames 60
ssTgt = ssHero
ssAimH = 1.8
ssOff = [4, 2.6, -9]
call ssAim
triSimFrames 12
triScreenshot "12_egyptian_armour"

ssOff = [4, 1.3, -6.5]
ssAimH = 1.4
call ssAim
triSimFrames 8
triScreenshot "13_t55a_low_angle"

ssTgt = units ssGrp select 1
ssOff = [1.0, 1.6, -3.4]
ssAimH = 1.5
call ssAim
triSimFrames 8
triScreenshot "14_frontier_corps"

call ssWipeMen
call ssWipe
triSimFrames 8

call {
    ssGrp = createGroup east;
    ssCls = ["LoBo_T72M_Syria", "LoBo_bmp_syr", "LoBo_BTR60_Syria"]; ssLx = 6900; ssLz = 9110; ssGap = -5.5; ssGap2 = 5; ssVDir = 135; call ssPark;
    ssMen = ["LoBo_WL_Infantry_Syria", "LoBo_WL_Infantry_SyriaMG1", "LoBo_WL_Infantry_SyriaRPG"];
    ssLx = 6897; ssLz = 9103; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
ssFace = 175
call ssPose
triSimFrames 60
ssTgt = ssHero
ssAimH = 1.8
ssOff = [5, 3.2, -11]
call ssAim
triSimFrames 12
triScreenshot "15_syrian_armour"

ssTgt = units ssGrp select 1
ssOff = [1.0, 1.6, -3.4]
ssAimH = 1.5
call ssAim
triSimFrames 8
triScreenshot "16_syrian_infantry"

call ssWipeMen
call ssWipe
triSimFrames 8

call {
    ssGrp = createGroup east;
    ssCls = ["LoBo_Challenger1_Jor_W", "LoBo_M60A3_jor", "LoBo_m113_jor", "LoBo_BMP2_Jor"]; ssLx = 6900; ssLz = 9110; ssGap = -5.5; ssGap2 = 5; ssVDir = 135; call ssPark;
    ssMen = ["LoBo_commando_jor", "LoBo_para_jor_mg", "LoBo_sniper_jor"];
    ssLx = 6897; ssLz = 9103; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
ssFace = 175
call ssPose
triSimFrames 60
ssTgt = ssHero
ssAimH = 2.0
ssOff = [5, 3.4, -12]
call ssAim
triSimFrames 12
triScreenshot "17_jordanian_armour"

ssOff = [4, 1.4, -7]
ssAimH = 1.5
call ssAim
triSimFrames 8
triScreenshot "18_challenger_low_angle"

ssTgt = units ssGrp select 0
ssOff = [1.0, 1.6, -3.4]
call ssAim
triSimFrames 8
triScreenshot "19_jordanian_commando"

call ssWipeMen
call ssWipe
triSimFrames 8

call { ssCls = ["LoBo_ZU23_terror", "LoBo_SA6", "LoBo_SA6RAD"]; ssLx = 6900; ssLz = 9110; ssGap = 11; ssGap2 = 9; ssVDir = 250; call ssPark }
triSimFrames 50
ssTgt = ssHero
ssAimH = 1.3
ssOff = [4, 1.7, -5]
call ssAim
triSimFrames 10
triScreenshot "20_zu23_emplacement"

ssTgt = ssJunk select 1
ssAimH = 2.0
ssOff = [7, 3.0, -9]
call ssAim
triSimFrames 8
triScreenshot "21_sa6_battery"

call ssWipe
triSimFrames 8

// ============================================================================
//  Refugee Camp - staged on the road SOUTH of the shanties, looking north so
//  the camp, its walls and the minaret sit behind the subject
// ============================================================================
setDate [1985, 6, 15, 15, 10]
call {
    ssGrp = createGroup resistance;
    ssCls = ["LoBo_Ter_Toy_R", "LoBo_Ural_Hizballah"]; ssLx = 11900; ssLz = 9808; ssGap = -12; ssGap2 = 1; ssVDir = 90; call ssPark;
    ssMen = ["LoBo_Terror_01R", "LoBo_Terror_RPGR", "LoBo_Terror_MGR", "LoBo_Terror_SVDR", "LoBo_Terror_02R"];
    ssLx = 11896; ssLz = 9802; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 5 }
ssFace = 190
call ssPose
triSimFrames 60
ssTgt = ssHero
ssAimH = 1.6
ssOff = [14, 5.5, -3]
call ssAim
triSimFrames 12
triScreenshot "22_technical_on_the_camp_road"

ssTgt = units ssGrp select 2
ssOff = [7, 2.6, -2]
ssAimH = 1.5
call ssAim
triSimFrames 8
triScreenshot "23_terror_cell"

ssTgt = units ssGrp select 1
ssOff = [0.8, 1.6, -3.2]
call ssAim
triSimFrames 8
triScreenshot "24_rpg_gunner"

call ssWipeMen
call ssWipe
triSimFrames 8

call {
    ssGrp = createGroup resistance;
    ssCls = ["LoBo_JeepMG_Hizballah", "LoBo_JeepAT_Hizballah"]; ssLx = 11900; ssLz = 9808; ssGap = -9; ssGap2 = 1; ssVDir = 90; call ssPark;
    ssMen = ["LoBo_HizballahLeader", "LoBo_HizballahRifle1", "LoBo_HizballahMG1", "LoBo_Hizballah_RPG", "LoBo_HizballahSniper1"];
    ssLx = 11896; ssLz = 9802; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 5 }
ssFace = 190
call ssPose
triSimFrames 60
ssTgt = units ssGrp select 2
ssAimH = 1.5
ssOff = [7, 2.6, -2]
call ssAim
triSimFrames 12
triScreenshot "25_hizballah_fighters"

ssTgt = ssHero
ssAimH = 1.4
ssOff = [12, 4.0, -3]
call ssAim
triSimFrames 8
triScreenshot "26_hizballah_gun_jeep"

ssAimH = 8
ssOff = [-40, 65, -120]
call ssAim
triSimFrames 12
triScreenshot "27_refugee_camp_establishing"

call ssWipeMen
call ssWipe
triSimFrames 8

// ============================================================================
//  As-Suways - the best street on the island
// ============================================================================
call {
    ssGrp = createGroup civilian;
    ssCls = ["LoBo_S1203", "LoBo_Uaz3741", "LoBo_Mazda6_Pol"]; ssLx = 1440; ssLz = 11030; ssGap = 8; ssGap2 = 6; ssVDir = 120; call ssPark;
    ssMen = ["LoBo_Civ_01", "LoBo_Civ_02", "LoBo_Civ_03", "LoBo_CivF_01"];
    ssLx = 1434; ssLz = 11022; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
call { { _x setCaptive true; _x setBehaviour "CARELESS"; _x setUnitPos "UP"; _x setDir 175; _x disableAI "MOVE" } forEach (units ssGrp) }
triSimFrames 60
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [1, 1.9, -6]
call ssAim
triSimFrames 12
triScreenshot "28_suways_street"

ssTgt = ssHero
ssAimH = 1.4
ssOff = [-3.5, 2.0, -7]
call ssAim
triSimFrames 8
triScreenshot "29_skoda_1203"

ssTgt = ssJunk select 2
ssOff = [-3.5, 1.8, -6]
call ssAim
triSimFrames 8
triScreenshot "30_police_mazda"

call ssWipeMen
call ssWipe
triSimFrames 8

// An IDF patrol in the same street - the urban half of Guerrilla Mode
call {
    ssGrp = createGroup west;
    ssCls = ["LoBo_HMWVUP_IDF_M240"]; ssLx = 1444; ssLz = 11034; ssGap = 0; ssGap2 = 0; ssVDir = 120; call ssPark;
    ssMen = ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWLAW"];
    ssLx = 1434; ssLz = 11022; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 175
call ssPose
triSimFrames 60
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [1, 1.9, -6]
call ssAim
triSimFrames 12
triScreenshot "31_idf_patrol_in_town"

ssAimH = 0
ssOff = [-60, 55, -70]
call ssAim
triSimFrames 12
triScreenshot "32_suways_establishing"

call ssWipeMen
call ssWipe
triSimFrames 8

// ============================================================================
//  Egyptian airbase - flightlines, tightened up
// ============================================================================
setDate [1985, 6, 15, 15, 40]
call { ssCls = ["LoBo_E_mig21bis_MR1", "LoBo_IL28_egy", "LoBo_Mi8_egy"]; ssLx = 3470; ssLz = 1600; ssGap = 17; ssGap2 = -4; ssVDir = 250; call ssPark }
triSimFrames 60
ssTgt = ssHero
ssAimH = 1.6
ssOff = [-11, 3.4, -15]
call ssAim
triSimFrames 12
triScreenshot "33_eaf_flightline"

ssOff = [-6, 1.6, -9]
ssAimH = 1.2
call ssAim
triSimFrames 8
triScreenshot "34_mig21_hero"

ssTgt = ssJunk select 1
ssAimH = 2.2
ssOff = [-12, 4.0, -16]
call ssAim
triSimFrames 8
triScreenshot "35_il28_beagle"

call ssWipe
triSimFrames 8

call { ssCls = ["LoBo_F16I_IAF_CAS", "LoBo_KfirC7CAS1", "LoBo_F15I_IAF_grey", "LoBo_A4"]; ssLx = 3470; ssLz = 1600; ssGap = 17; ssGap2 = -4; ssVDir = 250; call ssPark }
triSimFrames 60
ssTgt = ssHero
ssAimH = 1.6
ssOff = [-11, 3.4, -15]
call ssAim
triSimFrames 12
triScreenshot "36_iaf_flightline"

ssOff = [-6, 1.5, -9]
ssAimH = 1.2
call ssAim
triSimFrames 8
triScreenshot "37_f16i_sufa_hero"

ssTgt = ssJunk select 2
ssAimH = 1.8
ssOff = [-9, 2.6, -13]
call ssAim
triSimFrames 8
triScreenshot "38_f15i_raam"

call ssWipe
triSimFrames 8

// ============================================================================
//  Helipads south of El Tor
// ============================================================================
call { ssCls = ["LoBo_AH64A_IAF", "LoBo_UH60A_IAF"]; ssLx = 9096; ssLz = 2808; ssGap = 0; ssGap2 = 64; ssVDir = 200; call ssPark }
triSimFrames 60
ssTgt = ssHero
ssAimH = 1.6
ssOff = [-9, 2.8, 13]
call ssAim
triSimFrames 12
triScreenshot "39_iaf_rotary_wing"

ssOff = [-5, 1.3, 7.5]
ssAimH = 1.3
call ssAim
triSimFrames 8
triScreenshot "40_apache_peten_hero"

ssTgt = ssJunk select 1
ssOff = [-6, 1.6, 9]
ssAimH = 1.4
call ssAim
triSimFrames 8
triScreenshot "41_uh60_yanshuf"

call ssWipe
triSimFrames 8

// ============================================================================
//  Eilat - the container port and the skyline.  Camera well back and high:
//  an earlier pass put it inside a container stack.
// ============================================================================
setDate [1985, 6, 15, 16, 5]
call {
    ssCands = [[10198, 10098], [10248, 10098], [10298, 10098], [10198, 10048], [10348, 10098]];
    ssHero = objNull;
    ssI = 0;
    while "ssI < count ssCands" do {
        if (isNull ssHero) then {
            ssTry = "loBo_Snunit" createVehicle [(ssCands select ssI) select 0, (ssCands select ssI) select 1, 0];
            if (((getPosASL ssTry) select 2) < 1.0) then { ssHero = ssTry } else { deleteVehicle ssTry };
        };
        ssI = ssI + 1;
    };
    if (isNull ssHero) then { ssHero = "loBo_Snunit" createVehicle [10248, 10098, 0] };
    ssHero setDir 20;
    ssHero setCaptive true;
    ssJunk = [ssHero];
}
triSimFrames 60
ssTgt = ssHero
ssAimH = 0
ssOff = [-26, 34, -30]
call ssAim
triSimFrames 12
triScreenshot "42_snunit_missile_boat"

ssAimH = 0
ssOff = [-150, 110, -160]
call ssAim
triSimFrames 15
triScreenshot "43_eilat_port_establishing"

ssAimH = 25
ssOff = [-300, 70, -40]
call ssAim
triSimFrames 12
triScreenshot "44_eilat_skyline"

call ssWipe
triSimFrames 8

// ============================================================================
//  St Katherine - camera outside the monastery wall, looking north-west into
//  the palm oasis with the gate behind it
// ============================================================================
setDate [1985, 6, 15, 15, 50]
call {
    ssGrp = createGroup resistance;
    ssCls = ["LoBo_Ter_Toy_R"]; ssLx = 6528; ssLz = 6752; ssGap = 0; ssGap2 = 0; ssVDir = 300; call ssPark;
    ssMen = ["LoBo_Terror_01R", "LoBo_Terror_MGR", "LoBo_Terror_SVDR"];
    ssLx = 6524; ssLz = 6744; ssGap = 1.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
ssFace = 150
call ssPose
triSimFrames 60
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [6, 2.4, -9]
call ssAim
triSimFrames 12
triScreenshot "45_st_katherine_oasis"

ssTgt = ssHero
ssAimH = 6
ssOff = [55, 26, -60]
call ssAim
triSimFrames 12
triScreenshot "46_sinai_massif"

call ssWipeMen
call ssWipe
triSimFrames 8

triEndTest
