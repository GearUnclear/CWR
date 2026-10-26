// ============================================================================
//  Remastered lighting: Classic | Remastered capture pairs on Abel (Malden).
//  Not a regression test: a repeatable photo run, driven by run-captures.ps1
//  (see README.md beside this file for the view table and how to run it).
//
//  Every matched view runs the same block twice, Classic then Remastered:
//    switch the lighting model and restart the GPU / CPU timers  (capMode)
//    triSimFrames 90                    settle: shaders, shadow maps, streaming,
//                                       and enough frames for the smoothed timers
//    capReadTimers                      keep the timings of this mode
//    capPin                             same date, weather, sim clock, props, view
//    triSimFrames 3
//    triScreenshot "<view>__<mode>"     the presented frame (triCaptureAtPresent)
//    capRecord                          one UDCAP;event=shot log line
//  The remastered exposure meter is a pure function of the sun height
//  (LightRig.cpp), with no temporal adaptation, so nothing has to "settle" in
//  the tonemap between the two shots.
// ============================================================================

#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Abel"
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

// Capture settings shared by every view: 4x MSAA, 150% render scale (both
// applied at the next frame boundary), and the presented, post-gamma frame.
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]

// The horizon map (TerrainHorizon) is built off-thread when the island loads;
// give it time before the first frame that matters.
triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

// ----------------------------------------------------------------------------
//  1. lowsun_fp - ACCEPTANCE stand-in for the owner's LightingLowSun.Demo
//  (that mission was on the `demo` world, which the Remaster shadow does not
//  have). Open flat ground on north Malden [7300, 10900] (32 m, slope ~1 deg,
//  nothing within 20 m), first-person eye height, clear sky, 08:00 on 1 March:
//  sun 12.7 deg at az 117.5, 35 deg left of the 152.5 view heading. A Jeep, an
//  M113, a standing SoldierWB and the camouflage-net stand-in are set a few
//  metres apart, turned so each shows a sunlit face (normal ~az 45) and a
//  shaded face (normal ~az 315) to the camera. Remaster vanilla ships no
//  camouflage-net class (CONFIG.BIN, AddOns and DTA searched); the candidate
//  list takes @LoBo's LoBo_camnet when that mod is mounted and otherwise the
//  vanilla Soviet tent CampEast (cloth, half-lit material).
// ----------------------------------------------------------------------------
["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__classic"
"lowsun_fp__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "lowsun_fp__remastered"
"lowsun_fp__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  2. sunrise_ridge - south-west Malden [4400, 3300] (52 m), looking ENE (60)
//  at the shoulder of the central massif 2 km away (ridge line 4.2 deg at the
//  sun's azimuth). 05:08 on 5 July: sun 5.1 deg at az 64, just clear of the
//  ridge and 4 deg right of centre. Long view distance so the ridge is drawn.
// ----------------------------------------------------------------------------
["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__classic"
"sunrise_ridge__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "sunrise_ridge__remastered"
"sunrise_ridge__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  3. noon_fields - open upland on north Malden [6250, 10800] (115 m, slope
//  ~2 deg, nothing within 120 m), looking SSE (150) over fields with tree
//  lines at 150-800 m. 12:30 on 5 July: sun 71.9 deg at az 202. A Jeep and a
//  soldier in the foreground for short noon shadows and top-lit shading.
// ----------------------------------------------------------------------------
["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin
call {
    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;
    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__classic"
"noon_fields__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "noon_fields__remastered"
"noon_fields__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  4. forest_edge_golden - north-west coast [4674, 10156] (~45 m): open ground
//  with the sea to the west (no horizon toward the sun) and a wooded slope
//  rising 40-500 m to the east. Looking ENE (75) at the tree line, 18:15 on
//  5 July: sun 12.0 deg at az 290, behind the camera's left shoulder, so the
//  crowns facing the sea are sunlit and the far sides are in shade. A soldier
//  facing the sun stands at 12 m. (Run 1 stood 25 m further west, in a hollow
//  that the Remastered horizon map put in terrain shadow; moved onto the crest.)
// ----------------------------------------------------------------------------
["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin
call {
    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__classic"
"forest_edge_golden__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "forest_edge_golden__remastered"
"forest_edge_golden__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  5. coastline_sunset - west coast [1550, 5150] (shore at ~3.5 m, water
//  20 m west and open sea for 4 km along the sun's azimuth). Eye 3 m, looking
//  W (278), pitched down 4 deg so shore, surf and the sun path on the water
//  share the frame. 18:35 on 5 July: sun 8.4 deg at az 293, 15 deg right of
//  centre.
// ----------------------------------------------------------------------------
["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__classic"
"coastline_sunset__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "coastline_sunset__remastered"
"coastline_sunset__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  6. town_night - Saint Louis [7152, 8915], the Malden town with the most
//  street lamps (six lampazel.p3d within 60 m of [7143, 8975]). Eye height on
//  the southern approach looking NNW (345) up into the lamp cluster. 22:00 on
//  27 February: sun -50 deg, moon 68.7 deg at az 183 (moonIntensity 1, behind
//  the camera) for the moonlight fill. A campfire (class Fire, inflamed) at
//  9 m adds a near local light.
// ----------------------------------------------------------------------------
["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
call {
    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;
    if (not (isNull capFire)) then { capFire inflame true };
}
triSimFrames 150
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__classic"
"town_night__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
call { if (not (isNull capFire)) then { capFire inflame true } }
triSimFrames 3
triScreenshot "town_night__remastered"
"town_night__remastered" call capRecord
[] call capClear

// ----------------------------------------------------------------------------
//  7. firefight - NOT a matched pair. Open flat ground east of view 1
//  [7600, 10850] (29 m), camera looking north (0) at 2 m. 19:35 on 5 July,
//  sun -1.7 deg: dusk, so muzzle flashes, explosions and fire read as local
//  light. Four SoldierWB (west, left) and three SoldierEB (east, right) face
//  each other 22-28 m apart across the frame; everyone is captive, and each
//  man is ordered to doFire a man of the other line (the tools/screenshots
//  rig: reveal + per-man doFire aims level at a human). A Ural is destroyed
//  and kept burning behind the east line, and a 120 mm shell is dropped
//  30+ m behind the lines just before shots 1 and 2 (run 1 dropped them into
//  the gap and the blasts killed both squads before the Remastered half,
//  allowDamage false notwithstanding). The whole set is staged afresh for
//  each mode (capFFStage / capFFClear), so both modes start from the same
//  layout; AI fire and the sim clock are not re-pinned, so the flashes differ.
// ----------------------------------------------------------------------------
["firefight", 7600, 10850, 2, 0, 0, [1985, 7, 5, 19, 35], 0, 0, 1000] call capBegin
capPinTime = false
capFW = []
capFE = []
capBurnV = objNull
capFFStage = {
    capGrpFW = createGroup west;
    capGrpFE = createGroup east;
    east setFriend [west, 0];
    west setFriend [east, 0];
    capFW = [];
    capFE = [];
    { capNew = [["SoldierWB"], _x select 0, _x select 1, 90, "fighter", capGrpFW] call capSpawn; if (not (isNull capNew)) then { capFW = capFW + [capNew] } } forEach [[24, -15], [26, -12], [23, -9], [27, -6]];
    { capNew = [["SoldierEB"], _x select 0, _x select 1, -90, "fighter", capGrpFE] call capSpawn; if (not (isNull capNew)) then { capFE = capFE + [capNew] } } forEach [[25, 9], [28, 12], [24, 15]];
    capBurnV = [["Ural", "Truck5t"], 60, 24, 30, "veh"] call capSpawn;
    if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };
    format ["view=firefight;event=squads;west=%1;east=%2", count capFW, count capFE] call capLog;
}
capFFClear = {
    capAliveW = 0;
    capAliveE = 0;
    { if (alive _x) then { capAliveW = capAliveW + 1 } } forEach capFW;
    { if (alive _x) then { capAliveE = capAliveE + 1 } } forEach capFE;
    format ["view=firefight;event=survivors;west=%1;east=%2", capAliveW, capAliveE] call capLog;
    [] call capClear;
    capFW = [];
    capFE = [];
}
capEngage = {
    private ["_i"];
    if (((count capFW) > 0) and ((count capFE) > 0)) then {
        { (capFW select 0) reveal _x } forEach capFE;
        { (capFE select 0) reveal _x } forEach capFW;
        _i = 0;
        { _x doFire (capFE select (_i mod (count capFE))); _i = _i + 1 } forEach capFW;
        _i = 0;
        { _x doFire (capFW select (_i mod (count capFW))); _i = _i + 1 } forEach capFE;
    };
}
// [forward, right] call capBoom: a 120 mm shell dropped from 40 m into that spot.
capBoom = {
    private ["_p"];
    _p = _this call capRel;
    if ((count capFE) > 0) then { capShell = createShell ["Shell120", [_p select 0, _p select 1, 40], [0, 0, -250], capFE select 0, objNull] };
}
capBurn = { if (not (isNull capBurnV)) then { capBurnV inflame true } }
[] call capFFStage
"classic" call capMode
[] call capEngage
triSimFrames 200
[] call capReadTimers
call { [] call capPinEnv; [] call capPinView; [] call capEngage; [] call capBurn }
[62, -2] call capBoom
triSimFrames 5
triScreenshot "firefight_1__classic"
"firefight_1__classic" call capRecord
[] call capEngage
[58, -32] call capBoom
triSimFrames 6
triScreenshot "firefight_2__classic"
"firefight_2__classic" call capRecord
[] call capEngage
triSimFrames 20
triScreenshot "firefight_3__classic"
"firefight_3__classic" call capRecord
[] call capFFClear
triSimFrames 30
[] call capFFStage
"remastered" call capMode
[] call capEngage
triSimFrames 200
[] call capReadTimers
call { [] call capPinEnv; [] call capPinView; [] call capEngage; [] call capBurn }
[62, -2] call capBoom
triSimFrames 5
triScreenshot "firefight_1__remastered"
"firefight_1__remastered" call capRecord
[] call capEngage
[58, -32] call capBoom
triSimFrames 6
triScreenshot "firefight_2__remastered"
"firefight_2__remastered" call capRecord
[] call capEngage
triSimFrames 20
triScreenshot "firefight_3__remastered"
"firefight_3__remastered" call capRecord
[] call capFFClear

// ----------------------------------------------------------------------------
//  8-9. tank gunner: interior and optics. The player is put in the gunner seat
//  of an M60 at the view-1 spot [7300, 10900] facing 152.5, with an M113, a
//  Jeep and a soldier parked 50-70 m down the gun line. 10:00 on 5 July: sun
//  59.3 deg at az 114.6. The cutscene camera and triSetView are released here:
//  these are the player's own cameras (HUD included, identical in both modes).
//  INTERNAL is the vehicle interior view (the remastered post pass sees a
//  cleared world depth there, so no AO / shafts); GUNNER is the optic, a
//  screen overlay inside the world pass that turns bloom and shafts off.
//  The `cam=` field of each shot line records which camera was really active.
// ----------------------------------------------------------------------------
["tank_gunner_interior", 7300, 10900, 1.75, 152.5, 0, [1985, 7, 5, 10, 0], 0, 0, 1500] call capBegin
call {
    [["M113"], 70, 4, 90, "veh"] call capSpawn;
    [["Jeep"], 55, -10, -60, "veh"] call capSpawn;
    [["SoldierWB"], 50, 0, 180, "man"] call capSpawn;
}
call {
    capUseView = false;
    capCam cameraEffect ["terminate", "back"];
    capClearRes = triClearView;
    capTankCls = ["M60", "M1Abrams", "T72"] call capPick;
    capTank = objNull;
    if (capTankCls != "") then { capTank = capTankCls createVehicle [7300, 10900, 0] };
    if (not (isNull capTank)) then {
        capTank setDir 152.5;
        capTank setPos [7300, 10900, 0];
        capTank allowDamage false;
        player moveInGunner capTank;
    };
    format ["view=tank_gunner_interior;event=tank;class=%1;inTank=%2", capTankCls, not ((vehicle player) == player)] call capLog;
}
triSimFrames 60
call { (vehicle player) switchCamera "INTERNAL" }
triSimFrames 60
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "tank_gunner_interior__classic"
"tank_gunner_interior__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "tank_gunner_interior__remastered"
"tank_gunner_interior__remastered" call capRecord
call { capView = "tank_gunner_optics"; capSimT = time; (vehicle player) switchCamera "GUNNER" }
triSimFrames 60
"classic" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "tank_gunner_optics__classic"
"tank_gunner_optics__classic" call capRecord
"remastered" call capMode
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "tank_gunner_optics__remastered"
"tank_gunner_optics__remastered" call capRecord

call { "view=done;event=done" call capLog }
triEndTest
