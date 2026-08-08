// ============================================================================
//  Vanilla showcase - Malden (world "abel"), base game only, NO mods.
//
//  The @LoBo shoots (lobo_sinai_showcase / lobo_sinai_stories) cover the mod
//  side.  This is the same camera rig pointed at the stock 1985 Cold War
//  roster on a stock island, so the two sets can sit side by side: same engine,
//  same lighting, same framing discipline.
//
//  Read the header of lobo_sinai_showcase.test.sqf for the camera technique.
//  The short version:
//    * triSetView [east, UP, north, dirE, dirUp, dirN] does the framing, in
//      absolute world coordinates (engine axis order - Y is elevation).
//    * Camera positions are computed from `getPosASL obj` -> [east, north, ASL]
//      so no terrain height is ever hardcoded.
//    * A cutscene camera exists only to suppress the gameplay HUD, which is
//      gated on !GWorld->GetCameraEffect() (DisplayUIMenus.cpp:974).
//    * triScreenshot labels MUST be string literals - the runner parses them
//      to keep its own sequence counter.
//
//  SETS (from clustering abel.wrp's 49k objects offline with
//  `PoseidonTools terrain objects`, whose columns are [east, ELEVATION, north]):
//      Village      [7297, 7964] el 173  the largest brick-house village
//      Town         [8202, 3125] el  25  town square with a church
//      Farm         [6770, 6064] el 128  walled farmstead
//      Hilltop      [5533, 6975] el 337  chapel and a ruin, long views
//
//  Base-game weapon classes used for forced shots (from bin/config.bin):
//      SoldierWB "M16"   SoldierEB "AK74"   SoldierGB "AK47CZ"
//      T72/M1Abrams "Gun120"   T55G/M60 "Gun105"   ZSU "ZsuCannon"
//  Vehicle wreckage is always a real vehicle with setDammage 1 + inflame true.
// ============================================================================

triSimUntil { alive player }

setViewDistance 2500
0 setOvercast 0.1
0 setFog 0.02
showCinemaBorder false

// Park the player well clear of every set. [7150, 8970] is a village cluster,
// i.e. known dry land - an arbitrary coordinate on Malden can easily be sea,
// and a drowning player would end the mission mid-shoot.
player setPos [7150, 8970, 0]
player setCaptive true

ssCam = "camera" camCreate [7297, 7964, 30]
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

ssCivil = {
    { _x setCaptive true; _x setBehaviour "CARELESS"; _x setUnitPos "UP"; _x setDir ssFace; _x disableAI "MOVE" } forEach (units ssGrp)
}

ssShoot = { { _x fire (primaryWeapon _x) } forEach (units ssGrp) }

// A dead man leaves `units group`, so bodies are captured before they are
// killed or they leak into the next scene.
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
//  01-03  Soviet armour on the village road, late afternoon
// ============================================================================
setDate [1985, 6, 15, 16, 20]
call { ssCls = ["T72", "BMP", "Ural", "UAZ"]; ssLx = 7300; ssLz = 7940; ssGap = -5.5; ssGap2 = 5; ssVDir = 340; call ssPark }
triSimFrames 60
ssTgt = ssHero
ssAimH = 2.0
ssOff = [12, 3.4, -11]
call ssAim
triSimFrames 12
triScreenshot "01_soviet_column"
triAssertGt [(triGetPixelMaxChannel [0.5, 0.5]), 20]

ssOff = [7, 1.5, -7]
ssAimH = 1.6
call ssAim
triSimFrames 8
triScreenshot "02_t72_low_angle"

ssOff = [-4, 2.2, -12]
ssAimH = 1.8
call ssAim
ssHero fire "Gun120"
triSimFrames 1
triScreenshot "03_t72_main_gun"

call ssWipe
triSimFrames 8

// ============================================================================
//  04-06  Resistance ambush: a burning UAZ and the men who did it
// ============================================================================
call { ssCls = ["UAZ", "Ural"]; ssLx = 7300; ssLz = 7940; ssGap = -11; ssGap2 = 6; ssVDir = 340; call ssPark }
triSimFrames 40
call ssBurn
triSimFrames 50
ssBurnt = ssJunk

call {
    ssGrp = createGroup east;
    ssMen = ["SoldierEB", "SoldierEMG", "SoldierEMedic"];
    ssLx = 7306; ssLz = 7946; ssGap = -3.2; ssGapZ = -1.2;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
call ssKill
triSimFrames 40

call {
    ssGrp = createGroup resistance;
    ssMen = ["OfficerG", "SoldierGB", "SoldierGB", "SoldierGB"];
    ssLx = 7316; ssLz = 7934; ssGap = 2.0;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 250
call ssFight
triSimFrames 60
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [9, 2.4, -4]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "04_resistance_ambush"

call ssShoot
triSimFrames 1
triScreenshot "05_partisan_firing"

ssTgt = ssBurnt select 0
ssAimH = 1.6
ssOff = [10, 2.8, -7]
call ssAim
triSimFrames 8
triScreenshot "06_burning_uaz"

call ssWipeMen
call ssWipeBodies
call { { deleteVehicle _x } forEach ssBurnt; ssBurnt = [] }
ssJunk = []
triSimFrames 8

// ============================================================================
//  07-09  US rifle squad clearing the village street
// ============================================================================
setDate [1985, 6, 15, 15, 40]
call {
    ssGrp = createGroup west;
    ssCls = ["M113"]; ssLx = 7286; ssLz = 7976; ssGap = 0; ssGap2 = 0; ssVDir = 160; call ssPark;
    ssMen = ["OfficerW", "SoldierWB", "SoldierWG", "SoldierWLAW", "SoldierWMedic"];
    ssLx = 7294; ssLz = 7962; ssGap = 2.0;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 5 }
ssFace = 340
call ssFight
triSimFrames 60
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [3, 2.0, 9]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "07_us_squad_firing"

call ssShoot
triSimFrames 1
triScreenshot "08_rifleman"

ssTgt = ssHero
ssAimH = 1.6
ssOff = [8, 3.0, 10]
call ssAim
triSimFrames 8
triScreenshot "09_m113_and_squad"

call ssWipeMen
call ssWipe
triSimFrames 8

// ============================================================================
//  10-12  Civilians in the town square, and the church
// ============================================================================
setDate [1985, 6, 15, 15, 10]
call {
    ssGrp = createGroup civilian;
    ssCls = ["Skoda", "TruckV3SG"]; ssLx = 8210; ssLz = 3130; ssGap = 8; ssGap2 = 6; ssVDir = 100; call ssPark;
    ssMen = ["Civilian", "Civilian", "Civilian", "Civilian"];
    ssLx = 8204; ssLz = 3122; ssGap = 2.0;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 190
call ssCivil
triSimFrames 60
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [1, 2.0, -7]
call ssAim
triSimFrames 12
triScreenshot "10_town_square"

ssTgt = ssHero
ssAimH = 1.4
ssOff = [-4, 2.2, -8]
call ssAim
triSimFrames 8
triScreenshot "11_skoda_in_the_square"

ssAimH = 0
ssOff = [-70, 55, -80]
call ssAim
triSimFrames 12
triScreenshot "12_town_establishing"

call ssWipeMen
call ssWipe
triSimFrames 8

// ============================================================================
//  13-14  Cobra over the farm
// ============================================================================
setDate [1985, 6, 15, 16, 10]
call { ssCls = ["Cobra", "UH60"]; ssLx = 6780; ssLz = 6050; ssGap = -22; ssGap2 = 10; ssVDir = 200; call ssPark }
triSimFrames 60
ssTgt = ssHero
ssAimH = 1.6
ssOff = [-9, 2.6, -12]
call ssAim
triSimFrames 12
triScreenshot "13_cobra_and_blackhawk"

ssOff = [-5, 1.3, -7]
ssAimH = 1.3
call ssAim
triSimFrames 8
triScreenshot "14_cobra_hero"

call ssWipe
triSimFrames 8

// ============================================================================
//  15-16  Hilltop chapel, 337 m up - the long view over Malden
// ============================================================================
setDate [1985, 6, 15, 16, 15]
call {
    ssGrp = createGroup resistance;
    ssMen = ["OfficerG", "SoldierGB", "SoldierGB"];
    ssLx = 5540; ssLz = 6968; ssGap = 2.0;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
ssFace = 200
call ssStand
triSimFrames 60
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [2, 2.0, -8]
call ssAim
triSimFrames 12
triScreenshot "15_hilltop_watch"

ssAimH = 4
ssOff = [-45, 28, -60]
call ssAim
triSimFrames 12
triScreenshot "16_malden_vista"

call ssWipeMen
triSimFrames 8

// ============================================================================
//  17-19  Night contact by a burning truck
// ============================================================================
setDate [1985, 6, 15, 19, 10]
call { ssCls = ["Ural"]; ssLx = 7300; ssLz = 7940; ssGap = 0; ssGap2 = 0; ssVDir = 340; call ssPark }
triSimFrames 40
call ssBurn
triSimFrames 60
ssBurnt = ssJunk

call {
    ssGrp = createGroup west;
    ssMen = ["OfficerW", "SoldierWB", "SoldierWG", "SoldierWLAW"];
    ssLx = 7312; ssLz = 7934; ssGap = 1.9;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 250
call ssFight
triSimFrames 90
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [8, 2.2, -4]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "17_night_contact"

call ssShoot
triSimFrames 1
triScreenshot "18_muzzle_flash"

ssTgt = ssBurnt select 0
ssAimH = 1.6
ssOff = [12, 3.0, -8]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "19_firelight"

call ssWipeMen
call { { deleteVehicle _x } forEach ssBurnt; ssBurnt = [] }
triSimFrames 8

triEndTest
