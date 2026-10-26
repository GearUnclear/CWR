// look-ref landed reference, 960x640 sinai_lobo_lighting views (remastered block only): sinai_lowsun_fp,sinai_desert_noon
// ============================================================================
//  Remastered lighting: Classic | Remastered capture pairs on Sinai (@LoBo).
//  Same rig and per-mode block as abel_lighting.test.sqf; read that file's
//  header and capture_lib.sqf first. Mounts the shadow's own @LoBo (see the
//  .toml); the mission's addOns[] activates every @LoBo pbo the props need,
//  and each prop falls back to a vanilla class if its @LoBo class is missing.
//
//  SUN ON SINAI. @LoBo's CfgWorlds >> sinai sets latitude = 29.39. The engine
//  reads latitude with the opposite sign (DefaultWorld's -40 is 40 deg NORTH,
//  WorldInit.cpp / Lights.cpp), so the engine puts Sinai at 29.4 deg SOUTH:
//  the noon sun stands in the NORTH (38.8 deg at az 357 on 21 July, the
//  island's own startDate). Times and headings below are computed for what the
//  engine does (sunplan.py with latitude 29.39), not for the real Sinai.
// ============================================================================

#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Sinai"
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
capView = "setup"
capModeName = "none"
capLit = "none"
capGpu = "none"
capPerf = "none"

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [8350, 2950, 0]
capCam = "camera" camCreate [8350, 2950, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Sinai;msaa=4;scale=1.5;window=960x640;capture=present;lobo=%1", "LoBo_camnet" call capHas] call capLog }

// ----------------------------------------------------------------------------
//  1. sinai_lowsun_fp - the acceptance set again, on @LoBo content: the
//  Guerrilla.Sinai camp ground [8350, 2950] (34 m, slope ~4 deg, nothing
//  within 25 m), first-person eye height, clear sky. 16:10 on 21 July: sun
//  12.2 deg at az 303.8, 35 deg left of the 338.8 view heading, over the low
//  hills 0.6-1.2 km west. IDF HMMWV, M113 Zelda, Golani rifleman and @LoBo's
//  camouflage net (LoBo_camnet, LoBofightpos.pbo), turned like view 1 on Abel
//  so each shows a sunlit and a shaded face.
// ----------------------------------------------------------------------------
["sinai_lowsun_fp", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;
    [["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_lowsun_fp__remastered"
"sinai_lowsun_fp__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  2. sinai_desert_noon - open desert plateau in the north [7300, 10300]
//  (132 m, slope ~3 deg, nothing within 150 m), looking SSW (210) at the
//  mountains 2.4 km away (5.6 deg). 12:15 on 21 July: sun 38.8 deg at az 357,
//  behind the camera (see SUN ON SINAI). An IDF HMMWV and a rifleman in the
//  foreground for the sand-bounce and shade colour.
// ----------------------------------------------------------------------------
["sinai_desert_noon", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000] call capBegin
call {
    [["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"] call capSpawn;
    [["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sinai_desert_noon__remastered"
"sinai_desert_noon__remastered" call capRecord
[] call capClear

call { "view=done;event=done" call capLog }
triEndTest
