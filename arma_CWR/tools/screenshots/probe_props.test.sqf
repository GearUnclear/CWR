// Prop safety probe for the narrative shoot.
//
// ORIGINAL PURPOSE: several of these props used to take the process down with
// an 0xC0000005 - first in Building::DrawProxies (a dangling proxy in a
// 2005-era model), then, once that was guarded, in Building::Building itself
// (a partially resolved class loads as a bare EntityAIType, and the "house"
// constructor static_casts it to BuildingType).  So this spawns ONE candidate
// at a time, points the camera at it, renders, captures and deletes it: if the
// run died, the last screenshot written named the last SAFE prop and the next
// entry in the file was the culprit.
//
// CURRENT STATE (2026-08-08): all 21 entries complete and all 21 build.  A prop
// that cannot be built yields objNull and a WARN line instead of a crash, so the
// run being green does NOT by itself mean every prop appeared.  Grep
// game_stdout.log for
//   "Cannot create '<class>': type is abstract"
// and require zero hits.  The six LoBoWreck entries used to be refused: their
// pbo is one of two @LoBo addons that omit the #define public 2 header, so
// `scope = public;` read back as 0.  Repaired at source by
// tools/lobo/fix-lobo-scope.ps1 - if those six come back abstract, that script
// has not been run against this @LoBo install.
//
// p03/p03b are the other @LoBo content repair: both M60A1 wreck models were
// authored with their origin above the mesh, so createVehicle buried them
// (a static prop seats at terrainY + shape->BoundingCenter().Y) and the frame
// came back as empty desert.  Two layers now cover it:
// tools/lobo/fix-lobo-model-origin.ps1 repairs the p3d at source, and the
// engine seats any never-seatable model on its lowest vertex with a WARN
// (StaticSeatOffsetY, Entity::PlaceOnSurface).  An empty desert frame there
// means BOTH failed - which should not be possible on a current build.
//
// Keep the props in the same order as the list in the header comment so the
// mapping stays readable, and keep every triScreenshot label a string literal
// (the runner parses them to track its own sequence counter).

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

setViewDistance 2000
0 setOvercast 0.05
showCinemaBorder false
setDate [1985, 6, 15, 15, 30]

player setPos [2000, 6000, 0]
player setCaptive true

ssCam = "camera" camCreate [7900, 3200, 30]
ssCam cameraEffect ["internal", "back"]
triSimFrames 20

// An anchor object that is known good, so the camera always has something
// real to frame from even when the prop under test fails to build.
ssAnchor = "LoBo_Ter_Toy_R" createVehicle [11888, 9650, 0]
ssAnchor setDir 90
ssAnchor setCaptive true
triSimFrames 30

// The props all spawn at world [11900, 9650]; stand off ~18 m so a whole wreck
// fits the frame instead of filling it. triSetView takes raw engine [X, up, Z]
// order, so the SQF y coordinate is the THIRD component.
//
// WHY HERE. This is open ground: nothing placed in sinai.wrp sits within 70 m,
// while ~450 objects sit within 400 m, so it is unambiguously dry land on the
// edge of a built-up area rather than dune or sea. The earlier vantage at
// [11926, 9793] looked north straight through a lobo_israel_w_03 separation-wall
// segment on the n=9801 line: every frame came back as a full-bleed grey slab
// and proved nothing about whether the prop had drawn. If you move this scene,
// re-check for a wall between the camera and the spawn point
// (PoseidonTools terrain objects sinai.wrp, columns are [east, ELEVATION, north]).
ssAimAt = {
    ssT = getPosASL ssAnchor;
    triSetView [11913, (ssT select 2) + 5, 9637, -13, -3, 13]
}
call ssAimAt

// Each block: spawn beside the anchor, render, capture, delete.
ssP = "LoBo_uralwreck01" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p01_uralwreck01"
deleteVehicle ssP

ssP = "LoBo_uralwreck02" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p02_uralwreck02"
deleteVehicle ssP

ssP = "LoBo_M60A1_wreck" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p03_m60a1_wreck"
deleteVehicle ssP

ssP = "LoBo_M60A1_wreck2" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p03b_m60a1_wreck2"
deleteVehicle ssP

ssP = "LoBo_BTR60wreck1" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p04_btr60wreck1"
deleteVehicle ssP

ssP = "LoBo_Shot_Wreck1" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p05_shot_wreck1"
deleteVehicle ssP

ssP = "LoBo_wreck1" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p06_lobo_wreck1"
deleteVehicle ssP

ssP = "LoBo_wreck3" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p07_lobo_wreck3"
deleteVehicle ssP

ssP = "LoBo_Sandbag" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p08_sandbag"
deleteVehicle ssP

ssP = "LoBo_Sandbagx4" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p09_sandbag_x4"
deleteVehicle ssP

ssP = "LoBo_SandbagX8" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p10_sandbag_x8"
deleteVehicle ssP

ssP = "MAP_BarbedWire_4" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p11_barbedwire4"
deleteVehicle ssP

ssP = "MAP_BarbedFence" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p12_barbedfence"
deleteVehicle ssP

ssP = "MAP_Barrel1" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p13_barrel1"
deleteVehicle ssP

ssP = "MAP_Barrel2" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p14_barrel2"
deleteVehicle ssP

ssP = "GWRubble" createVehicle [11900, 9650, 0]
triSimFrames 8
triScreenshot "p15_gwrubble"
deleteVehicle ssP

ssP = "LoBo_building_08_ruin" createVehicle [11908, 9662, 0]
triSimFrames 8
triScreenshot "p16_building08_ruin"
deleteVehicle ssP

ssP = "LoBo_building_05_ruin" createVehicle [11908, 9662, 0]
triSimFrames 8
triScreenshot "p17_building05_ruin"
deleteVehicle ssP

ssP = "MAP_OilWell_Burning" createVehicle [11908, 9662, 0]
triSimFrames 20
triScreenshot "p18_oilwell_burning"
deleteVehicle ssP

ssP = "MAP_OilTurret_Burning" createVehicle [11908, 9662, 0]
triSimFrames 20
triScreenshot "p19_oilturret_burning"
deleteVehicle ssP

// A real vehicle destroyed in place - the safe fallback for wreckage if the
// static props above turn out to be unusable.
ssP = "LoBo_Ural_egy" createVehicle [11900, 9650, 0]
ssP setDir 90
ssP setDammage 1
ssP inflame true
triSimFrames 40
triScreenshot "p20_destroyed_ural"
deleteVehicle ssP

triEndTest
