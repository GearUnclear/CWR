// ============================================================================
//  VANILLA showcase - Nogova, base Cold War Assault assets, no mods at all.
//
//  Companion to lobo_sinai_stories.test.sqf.  Same rig, same rules; read the
//  header of lobo_sinai_showcase.test.sqf for the camera mechanics (triSetView
//  in absolute world coords, getPosASL to avoid hardcoding terrain height, the
//  cutscene camera that exists only to suppress the HUD, the string-literal
//  rule for triScreenshot labels, and the statement-splitting the harness does
//  to this file).  Everything there applies here unchanged.
//
//  WHY NOGOVA.  It is the greenest and most built-up of the base islands, so a
//  burning convoy on a forest road and a firefight between farmhouses look
//  nothing like the Sinai set - which is the point of shooting both.
//
//  WHERE THE SETS ARE.  Surveyed out of the world file the same way the Sinai
//  anchors were, but filtered on ROAD segments rather than object density,
//  because a road is the one piece of ground guaranteed clear enough to put a
//  camera and a convoy on:
//      PoseidonTools terrain objects noe.wrp       (cols: east, ELEV, north)
//  then asf*/sil* segments scored by how many house models sit within 50 m.
//
//    Forest road   [9737.9, 12489.8] el  16.5  86 trees within 60 m, no house
//                  within 150 m.  Runs NNE: the three segments are
//                  (9729.4,12466.3) (9737.9,12489.8) (9748.2,12548.2),
//                  bearing about 13 degrees.
//    Farm village  [3892.7,  7095.5] el 107.5  asf25 running due N-S at
//                  easting 3892.7 from north 7045 to 7120, nine houses within
//                  50 m - barns, a pub, a smithy, and two ruined cottages just
//                  north-east at [3908, 7089].
//    Block town    [3952.8,  5018.2] el  24.0  sil25 running due N-S at
//                  easting 3952.8, with thirteen panelak concrete blocks
//                  around [3959, 5028]; it crosses an E-W road at north 4974.4.
//
//  VANILLA CLASSES USED (all from bin/config.bin or AddOns/O.pbo, so nothing
//  here depends on an addon being activated):
//    men       SoldierGB/GMG/GLAW/GMedic/OfficerG, SoldierEB/EMG/ELAW/OfficerE,
//              SoldierWB/WMG/WLAW/WMedic, Civilian..Civilian11, Woman1..Woman5
//    vehicles  Ural, UAZ, BMP, T72, BRDM-less: T55G, UAZG, GJeep, TruckV3SG,
//              Skoda/SkodaBlue/SkodaRed, Jeep, M113, M1Abrams
//    guns      Gun120 (T72, T80 AND M1Abrams - read their weapons[], do not
//              assume Gun125 for the Soviet hull), Gun105 (M60), Gun73 (BMP)
//  Tank main guns need a CREW - `veh fire "Gun120"` on an empty hull does
//  nothing, so the armour scenes moveInDriver/moveInGunner first.
// ============================================================================

triSimUntil { alive player }

setViewDistance 2500
0 setOvercast 0.05
0 setFog 0.02
showCinemaBorder false

player setPos [7000, 9000, 0]
player setCaptive true

ssCam = "camera" camCreate [7000, 9000, 40]
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

ssLine = {
    ssI = 0;
    while "ssI < count ssMen" do {
        (ssMen select ssI) createUnit [[ssLx + ssI * ssGap, ssLz - ssI * ssGapZ, 0], ssGrp, "", 0.4, "PRIVATE"];
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

ssBurn = { { _x setDammage 1; _x inflame true } forEach ssJunk }

ssFight = {
    ssI = 0;
    while "ssI < count (units ssGrp)" do {
        ssU = (units ssGrp) select ssI;
        ssU setCaptive true;
        ssU setBehaviour "COMBAT";
        ssU setDir ssFace;
        ssU disableAI "MOVE";
        if (ssI % 3 == 1) then { ssU setUnitPos "MIDDLE" } else { if (ssI % 3 == 2) then { ssU setUnitPos "DOWN" } else { ssU setUnitPos "UP" } };
        ssI = ssI + 1;
    }
}

ssStand = {
    { _x setCaptive true; _x setBehaviour "COMBAT"; _x setUnitPos "UP"; _x setDir ssFace; _x disableAI "MOVE" } forEach (units ssGrp)
}

// Vanilla civilians will NOT stand still on their own, and the two obvious
// fixes both fail:
//   * per-unit setBehaviour "CARELESS" + setUnitPos "UP" is only a preference.
//     A real CIVILIAN (unlike @LoBo's civilians, which are reskinned soldier
//     classes and behave like soldiers) lies flat the moment it sees an armed
//     man or a burning car, and every civilian scene in pass 1 came back with
//     the villagers belly-down in the road.
//   * disableAI "ANIM" is worse, not better: with the animation selector gone
//     they freeze in the transition and hang half-sunk in the terrain.
// The ROOT cause turned out to be neither: Civilian4..Civilian11 and Woman1..5
// live in the BIS_Resistance addon, and until mission.sqm listed it in addOns[]
// the class only half-resolved and the unit had no usable stance at all.  With
// that fixed the group-level orders below are enough.
// What works is ordering the GROUP rather than the units - behaviour and combat
// mode live on the group in this engine (GrpSetBehaviour/GrpSetCombatMode) -
// and NOT calling disableAI "MOVE", which strands a civilian in whatever
// take-cover order it had already accepted.  Then shoot within a few frames of
// posing, before the group AI gets another tick.
ssCivil = {
    ssGrp setBehaviour "CARELESS";
    ssGrp setCombatMode "BLUE";
    { _x setCaptive true; _x disableAI "AUTOTARGET"; _x setUnitPos "UP"; _x setDir ssFace } forEach (units ssGrp)
}

// Re-assert stance and heading immediately before each frame.  Even with the
// addon resolved and the group told CARELESS, a civilian who can see a burning
// car or an armed man goes to hands and knees, so the last word is switchMove
// "Civil" - the standing civilian state in CfgMovesMC (CivilBase -> Civil), set
// directly on the model rather than requested through the AI.  disableAI "MOVE"
// goes with it so they do not simply crawl out of frame instead.
ssLock = {
    ssGrp setBehaviour "CARELESS";
    { _x disableAI "MOVE"; _x setUnitPos "UP"; _x setDir ssFace; _x switchMove "Civil" } forEach (units ssGrp)
}

ssShoot = { { _x fire (primaryWeapon _x) } forEach (units ssGrp) }
ssKill = { ssBodies = ssBodies + (units ssGrp); { _x setCaptive true; _x setDammage 1 } forEach (units ssGrp) }

ssWipe = { { deleteVehicle _x } forEach ssJunk; ssJunk = [] }
ssWipeMen = { { deleteVehicle _x } forEach (units ssGrp) }
ssWipeBodies = { { deleteVehicle _x } forEach ssBodies; ssBodies = [] }

ssJunk = []
ssBodies = []
ssGrp = grpNull
ssGap2 = 0
ssGapZ = 0.5

// ============================================================================
//  01-04  Forest-road ambush.  The convoy is strung NNE along the road with an
//  8 m gap - unit vector (0.224, 0.975) times 8 gives the per-vehicle step -
//  and the partisans are 15 m back down the same road, so a camera further
//  back again shoots straight up the road: fighters foreground, burning trucks
//  behind, trees closing both sides.
// ============================================================================
setDate [1985, 6, 15, 17, 45]
call { ssCls = ["Ural", "UAZ", "BMP"]; ssLx = 9730; ssLz = 12470; ssGap = 1.8; ssGap2 = 7.8; ssVDir = 13; call ssPark }
triSimFrames 40
call ssBurn
triSimFrames 50

// the convoy's escort, dead on the road beside the trucks
call {
    ssGrp = createGroup east;
    ssMen = ["SoldierEB", "SoldierEMG", "SoldierELAW"];
    ssLx = 9733; ssLz = 12476; ssGap = 1.4; ssGapZ = -3.6;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
call ssKill
triSimFrames 40

// the partisans, still firing up the road
call {
    ssGrp = createGroup resistance;
    ssMen = ["SoldierGB", "SoldierGLAW", "SoldierGMG", "OfficerG"];
    ssLx = 9727; ssLz = 12452; ssGap = 1.7; ssGapZ = -0.5;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 13
call ssFight
triSimFrames 60

ssTgt = units ssGrp select 2
ssAimH = 1.5
ssOff = [-1.6, 1.9, -7]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "01_forest_road_ambush"

call ssShoot
triSimFrames 1
triScreenshot "02_partisan_fire"

ssTgt = units ssGrp select 1
ssAimH = 1.2
ssOff = [-1.5, 1.2, -6]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "03_law_gunner_in_the_trees"

ssTgt = units ssGrp select 0
ssAimH = 1.5
ssOff = [3.5, 1.7, -3]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "04_partisan_firing"

call ssWipeMen
triSimFrames 8

// ============================================================================
//  05-07  Aftermath: a Soviet patrol comes up the road onto its own convoy
// ============================================================================
setDate [1985, 6, 15, 18, 10]
call {
    ssGrp = createGroup east;
    ssMen = ["OfficerE", "SoldierEB", "SoldierEMedic", "SoldierEMG"];
    ssLx = 9727; ssLz = 12453; ssGap = 2.0; ssGapZ = -0.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 13
call ssStand
triSimFrames 60

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [-2.0, 2.2, -8]
call ssAim
triSimFrames 8
triScreenshot "05_patrol_on_the_road"

ssTgt = units ssGrp select 3
ssAimH = 1.5
ssOff = [3.5, 1.7, -3.5]
call ssAim
triSimFrames 8
triScreenshot "06_at_the_burning_ural"

ssTgt = ssBodies select 1
ssAimH = 0.5
ssOff = [4.5, 1.4, -4]
call ssAim
triSimFrames 8
triScreenshot "07_the_escort"

call ssWipeMen
call ssWipeBodies
call ssWipe
triSimFrames 10

// ============================================================================
//  08-11  Firefight in the farm village.  The road runs due N-S at easting
//  3892.7 with barns and cottages on both sides; the men are staggered along
//  it and the camera sits south on the same line looking up the street.
// ============================================================================
setDate [1985, 6, 15, 16, 30]
call { ssCls = ["SkodaBlue", "TruckV3SCivil"]; ssLx = 3889; ssLz = 7098; ssGap = 5; ssGap2 = 9; ssVDir = 355; call ssPark }
triSimFrames 30
call ssBurn
triSimFrames 40

call {
    ssGrp = createGroup resistance;
    ssMen = ["SoldierGB", "SoldierGMG", "SoldierGLAW", "OfficerG"];
    ssLx = 3890; ssLz = 7078; ssGap = 0.9; ssGapZ = -1.9;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 0
call ssFight
triSimFrames 60

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [2.6, 1.9, -7]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "08_village_firefight"

call ssShoot
triSimFrames 1
triScreenshot "09_resistance_fire"

ssTgt = units ssGrp select 0
ssAimH = 1.5
ssOff = [2.0, 1.6, -2.6]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "10_partisan_rifleman"

ssTgt = units ssGrp select 3
ssAimH = 1.5
ssOff = [3.4, 1.8, -4.5]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "11_officer_firing"

call ssWipeMen
triSimFrames 8

// the other side of the same street
call {
    ssGrp = createGroup east;
    ssMen = ["SoldierEB", "SoldierEMG", "SoldierELAW", "OfficerE"];
    ssLx = 3890; ssLz = 7078; ssGap = 0.9; ssGapZ = -1.9;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 0
call ssFight
triSimFrames 60

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [2.6, 1.9, -7]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "12_soviets_in_the_village"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [3.4, 1.8, -4.5]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "13_machinegunner_firing"

call ssWipeMen
triSimFrames 8

// ============================================================================
//  14-16  Villagers in the street the fight went through.  Two ruined cottages
//  (domek_ruina) already stand at [3908, 7089], 20 m NE, so the wide frame gets
//  real rubble without spawning a single prop.
// ============================================================================
setDate [1985, 6, 15, 17, 0]
call {
    ssGrp = createGroup civilian;
    ssMen = ["Civilian4", "Woman2", "Civilian7", "Woman4"];
    ssLx = 3890; ssLz = 7080; ssGap = 1.1; ssGapZ = -2.1;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 180
call ssCivil
triSimFrames 30
call ssLock
triSimFrames 4

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [2.6, 1.9, -7]
call ssAim
call ssLock
triSimFrames 2
triScreenshot "14_villagers_in_the_street"

ssTgt = units ssGrp select 0
ssAimH = 1.5
ssOff = [2.0, 1.6, -2.6]
call ssAim
call ssLock
triSimFrames 2
triScreenshot "15_displaced"

ssTgt = units ssGrp select 2
ssAimH = 1.3
ssOff = [5.0, 3.0, -9]
call ssAim
call ssLock
triSimFrames 2
triScreenshot "16_leaving_the_village"

call ssWipeMen
call ssWipe
triSimFrames 10

// ============================================================================
//  17-19  Checkpoint among the concrete blocks
// ============================================================================
setDate [1985, 6, 15, 16, 45]
call { ssCls = ["BMP", "UAZ"]; ssLx = 3949; ssLz = 5024; ssGap = 6; ssGap2 = 5; ssVDir = 100; call ssPark }
triSimFrames 40

call {
    ssGrp = createGroup east;
    ssMen = ["OfficerE", "SoldierEB", "SoldierEMG"];
    ssLx = 3951; ssLz = 5016; ssGap = 1.8; ssGapZ = 1.2;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
ssFace = 175
call ssStand
triSimFrames 40
ssSoldier = units ssGrp select 1

call {
    ssGrp = createGroup civilian;
    ssMen = ["Civilian5", "Woman1", "Civilian9", "Civilian4"];
    ssLx = 3951; ssLz = 5010; ssGap = 1.5; ssGapZ = 1.3;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 0
call ssCivil
triSimFrames 30
call ssLock
triSimFrames 4

ssTgt = ssSoldier
ssAimH = 1.5
ssOff = [3.5, 2.2, -7]
call ssAim
triSimFrames 8
triScreenshot "17_checkpoint"

ssTgt = units ssGrp select 0
ssAimH = 1.5
ssOff = [2.4, 1.6, -3.0]
call ssAim
call ssLock
triSimFrames 2
triScreenshot "18_papers_please"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [5.0, 3.2, -9]
call ssAim
call ssLock
triSimFrames 2
triScreenshot "19_cordon_and_search"

call ssWipeMen
call ssWipe
triSimFrames 10

// ============================================================================
//  20-22  Armour in the block town.  Crewed, so the 125 mm actually fires.
// ============================================================================
setDate [1985, 6, 15, 18, 20]
call { ssCls = ["T72"]; ssLx = 3952; ssLz = 5020; ssGap = 0; ssGap2 = 0; ssVDir = 190; call ssPark }
triSimFrames 40
call {
    ssGrp = createGroup east;
    ssMen = ["SoldierEB", "SoldierEMG", "SoldierELAW", "OfficerE"];
    ssLx = 3948; ssLz = 5013; ssGap = 2.0; ssGapZ = 0.9;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
call { ssCrew = units ssGrp; (ssCrew select 0) moveInDriver ssHero; (ssCrew select 1) moveInGunner ssHero }
triSimFrames 40
ssFace = 190
call ssFight
triSimFrames 40

ssTgt = ssHero
ssAimH = 1.9
ssOff = [-9, 2.2, -7]
call ssAim
ssHero fire "Gun120"
triSimFrames 1
triScreenshot "20_t72_main_gun"

ssHero fire "MachineGun7_6"
triSimFrames 1
triScreenshot "21_tank_in_the_town"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [4.5, 1.7, -5]
call ssAim
ssHero fire "Gun120"
call ssShoot
triSimFrames 1
triScreenshot "22_infantry_beside_the_tank"

call ssWipeMen
call ssWipe
triSimFrames 10

// ============================================================================
//  23-25  Dusk contact on the forest road, lit by a burning truck
// ============================================================================
setDate [1985, 6, 15, 19, 5]
call { ssCls = ["Ural"]; ssLx = 9738; ssLz = 12490; ssGap = 0; ssGap2 = 0; ssVDir = 13; call ssPark }
triSimFrames 30
call ssBurn
triSimFrames 50

call {
    ssGrp = createGroup resistance;
    ssMen = ["SoldierGB", "SoldierGMG", "SoldierGLAW", "OfficerG"];
    ssLx = 9731; ssLz = 12464; ssGap = 1.7; ssGapZ = -0.5;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 13
call ssFight
triSimFrames 80

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [-1.4, 1.8, -6]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "23_dusk_contact"

call ssShoot
triSimFrames 1
triScreenshot "24_muzzle_flash"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [-2.4, 2.4, -10]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "25_firelight_on_the_road"

call ssWipeMen
call ssWipe
triSimFrames 10

triEndTest
