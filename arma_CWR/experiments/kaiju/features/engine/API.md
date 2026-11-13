# Engine support for the giant creature: script and config contract

Written by the engine agent. Source: `arma_CWR\engine\Poseidon\Game\Kaiju\` (plus small hooks in the
engine, listed in `kaiju-notes\features\engine\NOTES.md`). Everything here is off until a script
command or a config block switches it on for one unit, so stock content is unchanged.

Status, 2026-11-11. Proven in game on the proxy by `kj_engine.test.sqf` (3 runs of 3 on the final binary, about 90 checks each): far draw, camera
shake, light, emitter, steer, obstacle-blind planning, wading (the drowning check), damage ceiling,
hit reports, scripted damage, knockback, footfalls, memory points, the roar's reveal and flight,
detection, the lamp fix. Built but not proven in game: `gridRadius`, `noSpeedDamage`, `nearMip`,
the planner half of `wadeDepth`, the roar's late sound (every run is silent), the gaits other than
`walk`, `kjLight` with a memory point name, far draw on anything that is not a Man, and traits
given through the `class Kaiju` config block. Evidence and
limits: `D:\Arma_CWA_worktrees\kaiju-notes\features\engine\NOTES.md` and its `wip\` folder.

## The binary

- Game dir: `D:\Arma_CWA_worktrees\kaiju\arma_CWR\dist\kaiju-rwdi` (`PoseidonGame.exe`). Built from the
  kaiju worktree (origin/main `3df39c30` plus the uncommitted engine changes).
- Run a test on it by pointing the pipeline runner at it:
  `run-tri.ps1 -Test <test> -Tag <tag> -GameDir D:\Arma_CWA_worktrees\kaiju\arma_CWR\dist\kaiju-rwdi`
- The engine agent rebuilds this folder while it works. A build cannot replace an exe that is running,
  and a run started during a relink can pick up either build. If a run behaves oddly, run it again.

## Detecting it from script

A script cannot test for a command, and naming an unknown command is a script error on the stock
binary. The check below never names a command in a way the stock binary has to parse as one:

```
KJ_native = (format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_])
```

On the stock binary `kjVersion` is only an undefined variable, and `format` prints every undefined
value the same way, so the two sides are equal and `KJ_native` is false. On this binary it prints
`1`. Checked on both binaries (the stock run printed `scalar bool array string 0xfcffffef` for both
sides). `kj\engine\detect.sqs` beside this file does the same and also sets `KJ_engineVersion`.

Then guard every use. Both forms below are safe on the stock binary, because the engine keeps a
guarded statement and a `{ }` block as text and only parses it when it runs
(`Game/Scripting/Scripts.cpp:292-316`, `Evaluator/express.cpp:258`):

```
? KJ_native : kjShake [_pos, 1.5, 60, 400, 2.5]
if (KJ_native) then { kjShake [_pos, 1.5, 60, 400, 2.5] } else { [_pos] exec "kj\roar\shake.sqs" }
```

Never put a kaiju command on an unguarded line, or in a line whose guard is evaluated after the
command is parsed (for example as an argument of another command on the same line).

`kjVersion` is 2 since the finishing round of 2026-11-12 (1 before): 2 adds the traits `glow`,
`farGlow`, `plantBlend`, `pace`, `bodySink` and `lodPin`, the rate of `kjSteer`, `kjGround`, the LOD in
`kjFarState`, and the church collapse. A
script that needs one of them asks `kjVersion >= 2` inside a block the stock binary never runs. On
a version 1 binary `kjSet` answers false for the unknown trait and raises no error, so a script may
also simply set it.

## Rules that hold for every command

- Every command takes one argument (an array, or one object) and returns `false`, `-1`, `0` or `[]`
  on a malformed argument. None raises a script error, so a fallback path stays quiet.
- Positions given to a command are the usual `[x, y, z]` with z above the ground, or an object.
  Positions returned are `[x, y, z above sea level]`, the layout of `getPosASL`.
- Model-space offsets are `[right, forward, up]` in metres from the model origin (his feet).
- State is kept per object and dropped when the object is deleted. It is NOT saved with a save game:
  after a load, script-set traits, lights, emitters and steer orders are gone. Traits set from the
  config block come back by themselves, because the config is read when the unit is created.

## Traits: `kjSet [unit, "name", value]`, `kjGet [unit, "name"]`, `kjClear unit`

`kjSet` returns true, or false for an unknown name. `kjGet` returns the value, or -1 for an unknown
name. `kjClear` removes every trait, light, emitter and steer order of the unit.

| Trait | Value | What it does |
|---|---|---|
| `farDraw` | metres, 0 = off | The unit is drawn out to this distance, past the object distance and past the view distance, fogged down to a silhouette that thins toward the limit. It also keeps animating there. Works on any object, not only a Man. |
| `farContrast` | 0..1, default 0.5 | How strong the silhouette is at the edge of the fog. 1 = unfogged, 0 = invisible. |
| `wadeDepth` | metres, 0 = stock | Water depth the unit stands in without taking drowning damage (stock: 1.2 m, 0.6 m lying). The planner also lets it cross water: cells down to 2.5 m when `wadeDepth` is 2.5 or more, deeper cells when it is 10 or more (the planner grid does not know real depths past 2.5 m). |
| `obstacleBlind` | 0 or 1 | The path planner treats houses, trees, bushes and fully built or wooded cells as open ground for this unit. |
| `noSpeedDamage` | 0 or 1 | No crash damage for moving faster than 10 m/s on the ground. |
| `damageCeiling` | 0..1, negative = none | Total damage and every hit zone are held at or under this value, whatever hits. Scripted `setDammage 1` still kills at once; the ceiling catches weapons, explosions and `kjDamage`. |
| `hitReport` | 0 or 1 | Every hit is queued for `kjHits`, however small (the stock `Hit` event needs 0.05 total damage from one hit). |
| `stepShake` | strength, 0 = off | Every footfall starts a camera tremor at the foot (full within one body radius, gone at ten). |
| `gridRadius` | metres, 0 = stock | Widens every object-grid query from the stock 25 m margin to this radius while the unit exists, so parts further than 25 m from his origin (a tail, a reaching arm) can be hit and found. It makes every grid query on the map wider, so set it only as large as the model needs. |

| `nearMip` | 0 or 1 | Texture mip levels for the unit are asked for by the distance to the nearest point of its bounding sphere, not to its origin (his feet), as the landscape does for its own shapes. Standing at his chest then loads the sharp levels of the chest sheet. Built for the texture study; it replaces the hidden mip anchor triangles. NOT yet measured in game: the engine test only checks that the switch reads back. Measure it with `texres_charts` on this binary. |

Added in version 2 (the finishing round; evidence in `kaiju-notes\finishing\engine\NOTES.md`):

| Trait | Value | What it does |
|---|---|---|
| `glow` | 0..2, negative = stock (default) | Scale of the unit's shining (fullbright, "no light") faces, in the near pass and the far pass alike: 0 is black, 1 the painted colour at full brightness whatever the light (so it shows at night with no light near), above 1 the faces go white hot (added on top of the texture, to 2). With the trait set the unit also treats material 220 as shining: that is what the model loader makes of a face flagged "no light" whose points are flagged "no light" too, and the stock material table lights it like any face (the furnace of both final models is built that way, so without the trait it only LOOKS lit, by the sun or a light beside it). Stock units, and a unit with the trait at its default, are untouched. PROVEN: `kj_glow`. |
| `farGlow` | 0..1, default 0 | By night the fog floor of the far draw is raised toward this value (in step with the night, 0 by day), so the unit keeps its own colours past the view distance: an unlit hide stays black against the night, a lit furnace stays lit. The fade over the last 15 percent of `farDraw` is kept. PROVEN: `kj_glow` (from 2.3 km at a view distance of 2 km the furnace reads 229 of 255 with 0.9, 117 without). |
| `plantBlend` | 0 or 1 | While a move that starts at its first frame (`interpolationRestart = 1`) blends in from a move with no step (a stand), the body advances by the share of the step that keeps the standing foot where it is. Stock: the pose blend drags that foot back by about half the distance walked during the blend (1.03 m measured on creature A, 0.10 m with the trait). The same can be asked for one move with `plantBlend = 1;` in the move's class (it then holds for any unit that has a kaiju record). PROVEN: `kj_probe`. |
| `pace` | 0.2..2, default 1 | Rate of every move of the unit that travels (after the move's own `relSpeedMin` / `relSpeedMax`), so a walk can be slowed without a second clip. PROVEN under `kjSteer` only (`kj_probe`: 1.17 m/s at 0.5 against 2.34); the script set does not use it yet. |
| `bodySink` | metres, default 0 | The dead body is set this far into the ground (it falls there under the engine's own gravity). For a fall clip that leaves the trunk hanging above the ground; `kj\combat\settle.sqs` works the number out from the pose. No effect on a living unit. PROVEN: `kj_body`. |
| `lodPin` | 0 or 1 | The LOD the unit is drawn at follows distance alone: it is chosen at the finest LOD scale the scene allows, whatever the frame load has done to the current one. Stock: the scene's LOD scale grows with the frame time, which put the giant on his coarsest LOD a few hundred metres off in a busy town fight (the model round measured the far LODs coming in at 150 to 340 m on heavy frames, 1.0 to 2.4 km on light ones). Built and running in every gate run; NOT proven under a heavy frame (no test can make one on demand): on a light frame the two scales are the same. `kjFarState` now tells the LOD chosen (sixth element), so a recording can be checked. |

The same traits can be given in the unit's config, read when the unit is created:

```
class UDDedushka : Civilian
{
    class Kaiju
    {
        farDraw = 6000;
        farContrast = 0.5;
        wadeDepth = 30;
        obstacleBlind = 1;
        noSpeedDamage = 1;
        damageCeiling = 0.6;
        hitReport = 1;
        stepShake = 1.5;
        gridRadius = 40;
        nearMip = 1;
        // version 2, all optional:
        glow = 0.7;
        farGlow = 0.9;
        plantBlend = 1;
    };
};
```

On the stock binary the block is ignored (an unread class is not an error). The config path is
built but NOT yet proven in game: the pipeline's creature addon has no such block, and every trait
in the engine test was set with `kjSet`. If you build your own creature addon with the block, check
it with `kjGet` after creating the unit.

## Far draw

- `kjSet [unit, "farDraw", 6000]` and optionally `"farContrast"`. Nothing else is needed.
- `kjFarState unit` returns `[mode, distance, floor, framesNormal, framesFar, lod]` (`lod`, version 2: the
  LOD last chosen for drawing the unit, in the normal pass or the far pass; -1 before the first frame). Mode 0: off or past the
  limit. 1: inside the camera far plane, drawn by the normal pass, exempt from the object distance,
  with a fog floor. 2: past the far plane, drawn by the far pass. 3: past the far plane and off screen.
  `floor` is the lowest fog visibility the unit is allowed at that distance. The two frame counters
  count frames drawn each way, for tests.
- What it is not: the unit casts no shadow past the shadow distance, its proxies (weapons) still stop
  at the object distance, and nothing else is drawn out there. Terrain past the view distance is not
  drawn, so past it he is drawn in front of the sky and the horizon band and behind everything the
  scene does draw, with a band of ground haze over his legs: solid to just above his feet, gone at
  half his height. He rises out of the haze instead of standing in the air.
- By day the silhouette is his lit colour mixed toward the fog colour. At night it is dark on dark:
  only self-lit texture shows at a distance.
- The limit fades in: nothing pops at `farDraw` metres.

## Camera shake

- `kjShake [positionOrObject, strength, fullRadius, zeroRadius, seconds]`, optional sixth element: the
  rate in Hz (default 7; 4 to 5 reads heavier). Full strength inside `fullRadius` of the source,
  falling to nothing at `zeroRadius`; a 30 ms attack and a decay that ends when the time is up.
- Strength 1 is a firm shake (a few centimetres and about half a degree at the peak). Tremors add up;
  the total is capped at 3.
- It moves the camera, never the player: every camera is affected (first and third person, vehicles,
  optics, cutscene cameras, `triSetView`), and nothing in the simulation moves.
- `kjShakeGain` returns the total strength the camera feels right now (0 when calm).
- The roar feature's `kj\roar\shake.sqs` already calls it as `kjShake [pos, strength, fullRadius,
  zeroRadius, seconds]`: that matches, nothing to adapt.

## Light and particle emitter bound to a unit

- `kjLight [unit, "slot", point, [r, g, b], brightness]`. `point` is a memory point name (followed in
  the current pose) or a model offset `[right, forward, up]`. The colour may go above 1.
  `brightness` 1 lights at full strength out to 50 m and falls off with the square of the distance
  out to 500 m; the reach scales with it. Calling it again with the same slot changes the light in
  place (brighten it during the charge by calling it each frame or each step). `brightness` 0
  removes the slot. Returns true.
- `kjLightCount unit` returns the number of lights on the unit.
- It is a real point light: it lights the ground, buildings and units around it, at night only, the
  same as a campfire (the engine uses no lights by day). Terrain is lit per vertex, so the pool of
  light on open ground shows the facets of the 50 m terrain grid. Limits: 16 lights a scene, 7 an
  object, and a light further than the horizon distance plus 500 m from the camera is ignored.
- `kjEmitter [unit, "slot", dropArray, particlesPerSecond]`. `dropArray` is exactly what the `drop`
  command takes, in its 19 element form with the unit last, so the position may be a memory point
  name or a model offset and the velocity is in model space. The engine then drops that particle at
  the given rate (up to 200 a second) for as long as the unit lives and is within 3 km of the camera.
  A rate of 0 removes the slot (pass `[]` as the array).

## Steer and gait

- `kjSteer [unit, target, "walk" | "slow" | "fast" | "stop", arriveRadius, rate]`. `target` is a position or
  an object (walk there in a straight line) or a number (a heading in degrees, walk that way until
  told otherwise). `arriveRadius` is optional (default a quarter of his radius; 0 means the default).
  `rate` is optional (version 2, default 1): the rate the gait's clip plays at, 1 being the clip's own
  speed, held inside the move's `relSpeedMin` and `relSpeedMax`.
- It overrides the AI pilot: no path planning, no formation, no collision avoidance, no speed mode.
  It works with `disableAI "MOVE"` and `stop true` set, which is the cleanest way to use it. The unit
  turns at the turn rate of its moves class and walks only once it faces within 45 degrees.
- The gait picks the action of the moves class directly: `walk` is `walkF`, `slow` is `slowF`, `fast`
  is `fastF`. Ground speed comes from the clip at the order's rate. (Version 1 asked the pilot for
  1 m/s in the `walk` gait, so the walk clip played at its slowest allowed rate: 1.35 m/s measured
  where the clip's own speed is 2.35. Version 2, measured on creature A: `walk` 2.34 m/s, `slow`
  2.35 m/s, `walk` at rate 0.7 1.64 m/s.)
- `kjSteerState unit` returns `[active, arrived, metresLeft]` (`metresLeft` is -1 for a heading).
  After arrival the unit stands; the order stays until `kjSteerStop unit` or a new `kjSteer`.

## Damage

- `kjDamage [victim, owner, position, hit, range]`: explosion-style damage through the engine's own
  damage path, with no projectile and no effects. Armour, hit zones, `Hit`, `Dammaged` and `Killed`
  events all run, and `owner` (an object, or anything else for nobody) is credited. `position` is a
  position, an object, or `0` for the victim's own centre. `hit` and `range` mean what they mean in
  CfgAmmo (`indirectHit`, `indirectHitRange`). It works on map buildings and trees too, and does not
  log the "SetDammage applied on non-EntityAI" line.
- `kjHits unit` returns every hit since the last call, oldest first, and empties the queue (64 hits
  are kept). Each is `[attacker, "ammo class", modelPosition, damage, time, worldPosition]`. `damage`
  is what the hit added to total damage before the ceiling; `ammo class` is `""` for damage with no
  projectile. Needs the `hitReport` trait.
- `kjImpulse [object, [x, y, z]]`: a velocity change in m/s (world axes, z up), applied as an impulse
  through the physics, so a vehicle tumbles and a man is thrown. No projectile is involved.

## Footfalls, memory points, roar

- `kjSteps unit` returns `[count, lastWasLeft, position]`. The count goes up each time the playing
  move crosses one of its two sound edges (`soundEdge1`, `soundEdge2` in the move's config), whether
  or not the move plays a step sound. The engine reports edge 1 as the left foot. The position is the
  foot memory point (`stopaL` / `stopaP`) when the model has it, else the unit's position. Needs any
  trait set on the unit (for example `kjSet [unit, "stepShake", 0]`).
- `kjGround [x, y]` (version 2) returns `[height of the ground above sea level, metres of water over
  it]`, or `[]` on a malformed argument. The ground is the terrain itself, also under the sea: a
  script can otherwise only read the surface there (an object set on the sea floats), so wading had
  to feel its way a stride late. The brain reads the sea bed ahead with it on a retreat, and
  `kj\combat\settle.sqs` the ground under the chest of the falling body.
- `kjPoint [object, "memoryPoint"]` returns the world position of the point in the current pose, or
  `[]` when the model has no such point. `kjPoint [object, [right, forward, up]]` does the same for a
  model offset.
- `kjRoar [unit, "CfgSounds class", revealRadius, fearRadius, fearSeconds]` returns
  `[groupsRevealedTo, groupsFleeing]`. `fearSeconds` is optional (default 45). The sound (pass `""`
  for none) leaves his centre and arrives late with distance, as gunfire does. Every group whose
  leader is inside `revealRadius` learns exactly where he is (the same as `reveal`, at full
  accuracy). Every group whose leader is inside `fearRadius`, the player's own group excepted,
  breaks and runs away from him at once and stays in flight for `fearSeconds`, whatever its
  strength and courage; after that the engine's own rule decides again. It works on groups made by
  script too (the stock strength rule never lets those flee). To do that it moves the group's
  first waypoint to a point away from him, because that waypoint is where the engine flees to; a
  mission that needs the group's first waypoint afterwards must put it back.

## Stock crash fixed in this binary

- `lightIsOn` and `switchLight` on an object that is not a street lamp. On the stock binary every
  entity passed as a lamp (`StreetLamp` had no casting id of its own), `lightIsOn` of a soldier
  answered `"OFF"`, and `switchLight` wrote lamp state into him and crashed the game a frame later
  (found by the roar feature). Here `lightIsOn` answers `"ERROR"` for anything that is not a lamp
  and `switchLight` leaves it alone. The fix is in the binary, not in a command, so a script that
  must also run on the stock binary still has to recognise lamps by class name first.

## Crash at exit fixed (added by the integration, 2026-11-12)

- The game died on its way out (access violation, exit code -1073741819, after "Shutdown
  complete") whenever a `kjLight` was still bound to a unit: the static list of kaiju records was
  destroyed by the C runtime at exit, after the world its lights detach from was gone
  (`s_units` -> `Unit::~Unit` -> `KaijuLight::~KaijuLight` -> `World::RemoveAttachment`). Found by
  the integration's probe and reproduced three runs of three by the combat feature.
- Fix: `Kaiju::Shutdown()` (`Kaiju.hpp`, `KaijuService.cpp`) drops every record, and with it every
  light and emitter, and is called at the top of `World::~World` (`World\WorldSetup.cpp`) while the
  world still exists. Nothing changes for a script: no command was added.
- Proof: `experiments\kaiju\integration\tests\kj_exit` ends on purpose with the furnace light, the
  embers and every loop of the set still running; it exits cleanly on the rebuilt binary. A
  script no longer has to call `kjClear` before the mission ends (the tests still do, so that
  they also pass on a binary built before this).

## Finishing round, 2026-11-12 (version 2): what else changed

- **A destroyed church shows its collapse.** `Church::Animate` turned the clock hands and never
  called the base, where a destroyed object is morphed to its collapsed shape. With a kaiju record
  in the world a destroyed church now takes the base path (and leaves its hands alone); with none
  the stock behaviour is kept exactly. `World\Entities\Vehicles\House.cpp`. Proof: `kj_church`.
- **The far haze comes at once.** The band of ground haze in front of a unit drawn by the far pass
  grew from nothing at the far plane to full size over a third of a view distance, so just past the
  view distance his feet hung above the last drawn ridge. It now grows over a tenth of a view
  distance and is solid to his shins (`FarHazeBand`, `KaijuMath.hpp`).
- **Night.** See `glow` and `farGlow` above. The far pass itself was never at fault: the furnace
  faces were lit faces.

## Not built

- A beam primitive, a line damage query, burning objects, heat haze, looping sounds, object scale.
- A detail texture layer for the creature (the texture study's wish). Not built, and it is larger
  than the study's 8 lines for him: a skinned Man is drawn through the software T&L path (measured
  here: a state trace in the mesh draw call printed nothing for him while his 17 sections were
  drawn), so the mesh vertex shader that makes the landscape's detail UV set never sees his
  vertices. A detail layer for him has to fill the second UV set on the software path as well.
  Not verified further.
- Fear of size as a perception rule (the roar's fear radius is the scripted stand-in).
- An event handler for footfalls and hits (the two queues above are polled instead).
