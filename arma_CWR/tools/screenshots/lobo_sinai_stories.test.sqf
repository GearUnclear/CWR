// ============================================================================
//  @LoBo showcase, part 2 - NARRATIVE shots.
//
//  The sibling lobo_sinai_showcase.test.sqf is a roster catalogue: every
//  faction's armour, aircraft and infantry, parked and lit.  Useful, but a
//  parked tank is not a photograph.  This file stages moments instead -
//  weapons firing, a burning ambushed convoy, civilians in a wrecked street,
//  a checkpoint, an aftermath - using the same camera rig.
//
//  Read the header of lobo_sinai_showcase.test.sqf first: it documents
//  triSetView framing, the getPosASL coordinate trick, the HUD-suppressing
//  cutscene camera, the string-literal rule for triScreenshot labels, and
//  where each set is on the island.  All of that applies here unchanged.
//
//  WHAT IS DIFFERENT HERE:
//   * Firing.  `unit fire (primaryWeapon unit)` forces a shot regardless of
//     whether the AI has a target.  A muzzle flash lives about one frame, so
//     each firing scene fires the whole group, advances a single frame and
//     captures - then does it again for a second chance at the flash.
//     A VEHICLE weapon needs a gunner: `veh fire "gun"` on an empty tank does
//     nothing at all (pass 1 got two blank Merkava shots that way), so the
//     armour scenes crew the vehicle with moveInGunner/moveInDriver first.
//   * Wreckage is made from REAL VEHICLES, not from @LoBo's static wreck props.
//     `setDammage 1` gives the destroyed model and `inflame true` keeps it
//     alight; the props are a separate can of worms (see PROPS below).  A
//     killed Ural also throws a 40 m smoke column, which the static wreck
//     never does - it is the single best thing in these frames.
//   * Casualties.  setDammage 1 on a man leaves a body in the frame.  These are
//     setCaptive units that never fought - the damage is set directly, nothing
//     is shot at anything.  Dead men leave `units group`, so the bodies are
//     captured into ssBodies at spawn time and torn down from there.
//   * Posture.  setUnitPos "MIDDLE" crouches, "DOWN" goes prone; mixing them
//     across a group reads as a firefight rather than a parade.
//
//  FRAMING, learned in pass 1.  Aim at a MAN and put the burning thing behind
//  him, never the other way round: aiming at the wreck from 25 m turns the
//  fighters into specks on a runway.  Camera 6-12 m out and 1.6-2.4 m up, on
//  the line that runs subject -> background, is the whole trick.
//
//  PROPS.  Spawning @LoBo's static wrecks used to take the process down twice
//  over: first an 0xC0000005 in Building::DrawProxies (a dangling WeaponProxy
//  LLink - fixed in House.hpp/House.cpp), then one inside Building::Building
//  itself.  The second was a type-confusion: an addon-denied config entry
//  resolves `scope` through to the inherited 0, VehicleTypeBank::Load then
//  hands back a bare abstract EntityAIType, and NewVehicle static_cast it to
//  BuildingType anyway - NPos() read garbage and _locks.Resize() memmove'd it.
//  NewVehicle now refuses to build an abstract type (VehicleTypes.cpp), so a
//  denied prop is a no-show instead of a crash.  This file therefore never
//  aims a camera AT a prop and never depends on one being there.
//
//  WHERE THE SETS ARE.  Two anchors from the roster shoot are deliberately not
//  used: the Refugee Camp road at [11900, 9808] puts the camera inside solid
//  geometry from every offset tried (shots 22-26 of the roster run came back
//  blank grey), and the helipads have nothing but apron.  The three used here:
//      As-Suways        [ 1440, 11030]  the only real street on the island.
//                                       The camera works at eye level SE of a
//                                       subject at [1436, 11022] looking NW up
//                                       the street; the reverse angle looks
//                                       down an empty road and the NE side
//                                       puts the lens inside a shop wall.
//      Egyptian F.B.    [ 6900,  9110]  ridge-top base on a paved road - the
//                                       one long clear road stretch
//      Israeli F.B.     [ 9330, 11310]  fence, towers and revetments
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

setViewDistance 3500
0 setOvercast 0.05
0 setFog 0.03
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

// Burn everything currently parked.  setDammage swaps in the destroyed model
// and starts the wreck smoking; inflame keeps the flame going long enough that
// the shot does not have to be taken in the first half second.
ssBurn = { { _x setDammage 1; _x inflame true } forEach ssJunk }

// Everyone up, weapons ready, facing ssFace - then the middle third crouches
// and the last third goes prone, so the line reads as a fight.
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

// Same, but everyone standing - for patrols and checkpoint guards, where a
// prone man just disappears into the ground.
ssStand = {
    { _x setCaptive true; _x setBehaviour "COMBAT"; _x setUnitPos "UP"; _x setDir ssFace; _x disableAI "MOVE" } forEach (units ssGrp)
}

ssCivil = {
    { _x setCaptive true; _x setBehaviour "CARELESS"; _x setUnitPos "UP"; _x setDir ssFace; _x disableAI "MOVE" } forEach (units ssGrp)
}

ssShoot = { { _x fire (primaryWeapon _x) } forEach (units ssGrp) }

// Kill the current group and remember it - a dead man is no longer in
// `units group`, so ssWipeMen would leak the bodies into the next scene.
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
//  01-04  Convoy ambush on the ridge road, Egyptian Fire Base
//
//  The convoy runs NW up the road from [6900, 9110]; the ambush party is 12 m
//  further SE along the same road, so the camera can sit SE again and shoot
//  straight up the road with the fighters filling the foreground and the
//  burning trucks behind them.  Everything stays in the road corridor, which
//  is the only ground here verified clear.
// ============================================================================
setDate [1985, 6, 15, 15, 50]
call { ssCls = ["LoBo_Ural_egy", "LoBo_UAZ_Egy", "LoBo_BMP_EGY"]; ssLx = 6900; ssLz = 9110; ssGap = -5.5; ssGap2 = 5; ssVDir = 315; call ssPark }
triSimFrames 40
call ssBurn
triSimFrames 50

// the convoy's escort, dead on the verge beside the trucks
call {
    ssGrp = createGroup east;
    ssMen = ["LoBo_Egypt_FrtCrp", "LoBo_Egypt_FrtCrpMG", "LoBo_Egypt_FrtCrpAT"];
    ssLx = 6899; ssLz = 9116; ssGap = -3.4; ssGapZ = -1.2;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
call ssKill
triSimFrames 40

// the ambush party, still shooting up the road
call {
    ssGrp = createGroup resistance;
    ssMen = ["LoBo_Terror_01R", "LoBo_Terror_RPGR", "LoBo_Terror_MGR", "LoBo_Terror_02R"];
    ssLx = 6906; ssLz = 9102; ssGap = 2.2; ssGapZ = -0.6;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 315
call ssFight
triSimFrames 60

// aimed at the third fighter, camera on the fighter->convoy line, so the
// burning trucks sit behind the group instead of swallowing it
ssTgt = units ssGrp select 2
ssAimH = 1.5
ssOff = [11, 2.1, -9]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "01_convoy_ambush"

call ssShoot
triSimFrames 1
triScreenshot "02_ambush_fire"

// down at prone-gunner height
ssTgt = units ssGrp select 1
ssAimH = 1.0
ssOff = [7, 1.0, -6]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "03_rpg_gunner_in_the_fight"

ssTgt = units ssGrp select 0
ssAimH = 1.5
ssOff = [4.5, 1.7, -3.5]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "04_ambusher_firing"

call ssWipeMen
triSimFrames 8

// ============================================================================
//  05-07  Aftermath: an IDF patrol walks up on the burnt-out convoy
// ============================================================================
setDate [1985, 6, 15, 16, 15]
call {
    ssGrp = createGroup west;
    ssMen = ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMedic", "LoBoGolaniWMG"];
    ssLx = 6906; ssLz = 9103; ssGap = 2.4; ssGapZ = -0.7;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 315
call ssStand
triSimFrames 60

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [8, 2.2, -6.5]
call ssAim
triSimFrames 8
triScreenshot "05_patrol_finds_the_convoy"

ssTgt = units ssGrp select 3
ssAimH = 1.5
ssOff = [4.5, 1.7, -3.5]
call ssAim
triSimFrames 8
triScreenshot "06_at_the_wreck"

// down on the escort, wrecks still smoking behind them
ssTgt = ssBodies select 1
ssAimH = 0.5
ssOff = [6, 1.4, -5]
call ssAim
triSimFrames 8
triScreenshot "07_the_escort"

call ssWipeMen
call ssWipeBodies
triSimFrames 8

// ============================================================================
//  08-10  Armour on the road: a Merkava at the ambush site.  CREWED, so the
//  main gun actually fires - an empty hull ignores `fire` completely.
// ============================================================================
setDate [1985, 6, 15, 16, 20]
call { ssCls = ["LoBoMerkava2"]; ssLx = 6912; ssLz = 9096; ssGap = 0; ssGap2 = 0; ssVDir = 315; call ssPark }
triSimFrames 40
call {
    ssGrp = createGroup west;
    ssMen = ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWLAW"];
    ssLx = 6906; ssLz = 9101; ssGap = 2.3; ssGapZ = -0.7;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 315
call ssFight
triSimFrames 40
// first man drives, second gunners; the rest stay dismounted alongside
call { ssCrew = units ssGrp; (ssCrew select 0) moveInDriver ssHero; (ssCrew select 1) moveInGunner ssHero }
triSimFrames 40

ssTgt = ssHero
ssAimH = 1.9
ssOff = [10, 2.4, -8]
call ssAim
ssHero fire "LoBo_105L7A"
triSimFrames 1
triScreenshot "08_merkava_main_gun"

ssHero fire "LoBo_105L7A"
triSimFrames 1
triScreenshot "09_tank_supporting_the_advance"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [6, 1.6, -5]
call ssAim
ssHero fire "LoBo_105L7A"
call ssShoot
triSimFrames 1
triScreenshot "10_infantry_beside_the_tank"

call ssWipeMen
call ssWipe
triSimFrames 10

// ============================================================================
//  11-14  Street fight in As-Suways.  The only street on the island with real
//  facades on both sides.  Pass 1 proved the SE-looking-NW angle at eye level;
//  the burning car goes UP the street on that same line so it lands in the
//  background of every frame instead of off the edge.
// ============================================================================
setDate [1985, 6, 15, 15, 25]
call { ssCls = ["LoBo_S1203", "LoBo_Mazda6_Pol"]; ssLx = 1428; ssLz = 11030; ssGap = -6; ssGap2 = 5; ssVDir = 120; call ssPark }
triSimFrames 30
call ssBurn
triSimFrames 40

call {
    ssGrp = createGroup resistance;
    ssMen = ["LoBo_HizballahRifle1", "LoBo_HizballahMG1", "LoBo_Hizballah_RPG", "LoBo_HizballahLeader"];
    ssLx = 1434; ssLz = 11022; ssGap = 1.9; ssGapZ = 0.5;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 300
call ssFight
triSimFrames 60

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [4, 1.8, -4]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "11_street_firefight"

call ssShoot
triSimFrames 1
triScreenshot "12_insurgent_fire"

ssTgt = units ssGrp select 0
ssAimH = 1.5
ssOff = [1.8, 1.6, -2.4]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "13_rifleman_firing"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [5.5, 1.8, -4.5]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "14_rpg_team_in_the_street"

call ssWipeMen
triSimFrames 8

// the IDF section holding the same street, shot from the same working side
call {
    ssGrp = createGroup west;
    ssMen = ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWLAW"];
    ssLx = 1434; ssLz = 11022; ssGap = 1.9; ssGapZ = 0.5;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 300
call ssFight
triSimFrames 60

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [4, 1.8, -4]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "15_idf_in_the_street"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [5.5, 1.8, -4.5]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "16_golani_firing"

call ssWipeMen
triSimFrames 8

// ============================================================================
//  17-19  Civilians in the street the fight went through
// ============================================================================
setDate [1985, 6, 15, 16, 5]
call {
    ssGrp = createGroup civilian;
    ssMen = ["LoBo_Civ_01", "LoBo_CivF_01", "LoBo_Civ_02", "LoBo_Civ_03"];
    ssLx = 1434; ssLz = 11023; ssGap = 2.1; ssGapZ = 0.8;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 300
call ssCivil
triSimFrames 60

ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [4, 1.8, -4]
call ssAim
triSimFrames 8
triScreenshot "17_civilians_in_the_street"

ssTgt = units ssGrp select 0
ssAimH = 1.5
ssOff = [1.9, 1.6, -2.4]
call ssAim
triSimFrames 8
triScreenshot "18_displaced"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [6.5, 2.4, -5.5]
call ssAim
triSimFrames 8
triScreenshot "19_walking_out"

call ssWipeMen
triSimFrames 8

// ============================================================================
//  20-22  Checkpoint, same street, same camera side.  Pass 1 staged this at
//  [1444, 11034] and put half of every frame inside a shop wall; it is now on
//  the [1434, 11022] line that works, with the Humvee across the top of it.
// ============================================================================
setDate [1985, 6, 15, 15, 40]
call ssWipe
triSimFrames 8
call { ssCls = ["LoBo_HMWVUP_IDF_M240"]; ssLx = 1428; ssLz = 11029; ssGap = 0; ssGap2 = 0; ssVDir = 30; call ssPark }
triSimFrames 40

call {
    ssGrp = createGroup west;
    ssMen = ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG"];
    ssLx = 1432; ssLz = 11025; ssGap = 2.0; ssGapZ = 1.2;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 3 }
ssFace = 120
call ssStand
triSimFrames 40
ssSoldier = units ssGrp select 1

call {
    ssGrp = createGroup civilian;
    ssMen = ["LoBo_Civ_01", "LoBo_Civ_02", "LoBo_CivF_01", "LoBo_Civ_03"];
    ssLx = 1437; ssLz = 11020; ssGap = 1.5; ssGapZ = 1.3;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 300
call ssCivil
triSimFrames 60

ssTgt = ssSoldier
ssAimH = 1.5
ssOff = [6, 2.2, -5]
call ssAim
triSimFrames 8
triScreenshot "20_checkpoint"

ssTgt = units ssGrp select 0
ssAimH = 1.5
ssOff = [3.0, 1.6, -2.6]
call ssAim
triSimFrames 8
triScreenshot "21_papers_please"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [8, 3.0, -6.5]
call ssAim
triSimFrames 8
triScreenshot "22_checkpoint_wide"

call ssWipeMen
call ssWipe
triSimFrames 10

// ============================================================================
//  23-25  Night contact at the Israeli Fire Base, lit by a burning truck.
//  18:45 in pass 1 was almost pitch black; 18:15 still reads as night on this
//  world but keeps enough sky to separate the men from the ground.
// ============================================================================
setDate [1985, 6, 15, 18, 15]
call { ssCls = ["LoBo_Ural_egy"]; ssLx = 9330; ssLz = 11310; ssGap = 0; ssGap2 = 0; ssVDir = 75; call ssPark }
triSimFrames 30
call ssBurn
triSimFrames 50

call {
    ssGrp = createGroup west;
    ssMen = ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWLAW"];
    ssLx = 9338; ssLz = 11305; ssGap = 1.9; ssGapZ = 0.6;
    call ssLine;
}
triSimUntil { (count (units ssGrp)) >= 4 }
ssFace = 305
call ssFight
triSimFrames 80

// on the men->truck line, far enough back that the fire is in shot behind them
ssTgt = units ssGrp select 1
ssAimH = 1.5
ssOff = [9, 2.4, -7]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "23_night_contact"

call ssShoot
triSimFrames 1
triScreenshot "24_muzzle_flash"

ssTgt = units ssGrp select 2
ssAimH = 1.4
ssOff = [14, 3.4, -11]
call ssAim
call ssShoot
triSimFrames 1
triScreenshot "25_firelight"

call ssWipeMen
call ssWipe
triSimFrames 10

triEndTest
