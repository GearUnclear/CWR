// ============================================================================
//  Chupacabra model spawn (experiment; mission experiments/cryptid/ChupaModel.Eden,
//  mod @udchupa from experiments/cryptid/addon/build-udchupa.ps1).
//  First in-game load of an MLOD written by the Blender exporter: the runtime
//  loader (LODShape::Load / Shape::LoadTagged), the creature's rest space on
//  the stock man skeleton, and how the stock CfgMovesMC moves read on it.
//  Hard asserts: the UDChupacabra unit is created, alive, and walks and runs
//  (it moves). The poses (crouch, prone, crawl) are logged as CHUPA lines and
//  photographed, not asserted: Civilian's moves have no crouch action, so the
//  crouch is a forced switchMove and may not hold.
//  Daylight (10:00), quiet: no weapons on anyone, nobody fires.
//  Screenshots: tmp/tri/<id>/cryptid_model_spawn/*.png, camera a few metres
//  off the creature, re-aimed before every shot.
// ============================================================================
player allowDammage false
player setCaptive true
removeAllWeapons player
// Out of frame: behind and left of every camera position used below.
player setPos [4760, 10930, 0]
setDate [1985, 5, 10, 10, 0]
triAssert [(dayTime > 9) and (dayTime < 11)]
chC = createCenter civilian
chG = createGroup civilian
chupa = objNull
chStart = [4800, 11000, 0]
"UDChupacabra" createUnit [chStart, chG, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull chupa)]
triAssertEq [typeOf chupa, "UDChupacabra"]
triAssert [alive chupa]
chupa setDir 90
chupa addEventHandler ["AnimChanged", {logInfo format ["CHUPA anim %1 %2", time, _this select 1]}]
logInfo format ["CHUPA spawned pos %1 move %2 weapons %3 mags %4 side %5", getPos chupa, getMove chupa, weapons chupa, magazines chupa, side chupa]
// Camera 3/4 front-right: south-east of the creature, looking north-west at
// mid-body. triSetView takes world [x, height ASL, north, dx, dy, dz].
chView = { private ["_p"]; _p = getPosASL chupa; [(_p select 0) + 3.5, (_p select 2) + 1.5, (_p select 1) - 4.5, -3.5, -0.75, 4.5] }
// Side on, from the south, a little further back for the moving shots.
chSide = { private ["_p"]; _p = getPosASL chupa; [(_p select 0), (_p select 2) + 1.3, (_p select 1) - 6.5, 0, -0.35, 6.5] }
// Head close-up from the front (the creature faces east after setDir 90).
chFace = { private ["_p"]; _p = getPosASL chupa; [(_p select 0) + 2.2, (_p select 2) + 1.7, (_p select 1) - 0.6, -2.2, -0.15, 0.6] }
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
// -- standing ---------------------------------------------------------------
triSetView ([] call chView)
triSimFrames 3
triScreenshot "01_stand_34"
chupa setDir 90
triSimFrames 3
triSetView ([] call chFace)
triSimFrames 3
triScreenshot "02_stand_face"
// -- walk -------------------------------------------------------------------
chupa setSpeedMode "LIMITED"
chupa doMove [4818, 11000, 0]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA walk move %1 speed %2 pos %3", getMove chupa, speed chupa, getPos chupa]
triSetView ([] call chSide)
triSimFrames 3
triScreenshot "03_walk_side"
triAssert [(chupa distance chStart) > 1]
// -- crouch: setUnitPos knows only Up/Down/Auto here and Civilian's actions
//    have no crouch, so force the stock kneel state and hold the AI still ----
chupa stop true
chupa disableAI "MOVE"
if (true) then { t0 = time }
triSimUntil { time > t0 + 1 }
chupa switchMove "Crouch"
if (true) then { t0 = time }
triSimUntil { time > t0 + 1.5 }
logInfo format ["CHUPA switchMove Crouch move %1", getMove chupa]
triSetView ([] call chView)
triSimFrames 3
triScreenshot "04_crouch_34"
// -- prone and crawl ----------------------------------------------------------
chupa switchMove "Civil"
chupa enableAI "MOVE"
chupa stop false
chupa setUnitPos "DOWN"
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA prone move %1", getMove chupa]
triSetView ([] call chView)
triSimFrames 3
triScreenshot "05_prone_34"
chupa doMove [4826, 11000, 0]
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA crawl move %1 speed %2 pos %3", getMove chupa, speed chupa, getPos chupa]
triSetView ([] call chSide)
triSimFrames 3
triScreenshot "06_crawl_side"
// -- run back west, over ground the cryptid probe already crossed -------------
chupa setUnitPos "UP"
chupa setSpeedMode "FULL"
chRunFrom = getPos chupa
chupa doMove [4780, 11000, 0]
if (true) then { t0 = time }
triSimUntil { time > t0 + 3.5 }
logInfo format ["CHUPA run move %1 speed %2 pos %3", getMove chupa, speed chupa, getPos chupa]
triSetView ([] call chSide)
triSimFrames 3
triScreenshot "07_run_side"
triSetView ([] call chView)
triSimFrames 3
triScreenshot "08_run_34"
triAssert [(chupa distance chRunFrom) > 3]
triAssert [alive chupa]
logInfo format ["CHUPA end time %1 damage %2 move %3", time, damage chupa, getMove chupa]
triEndTest
