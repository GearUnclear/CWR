# Engine wishes from the anim feature

For the engine agent. Each item says what the animation has to fake or give up today, where the engine does it (paths under `arma_CWR\engine\Poseidon\`, line numbers of the unmodified checkout `3df39c30`; the engine agent's own edits shift them), and the smallest change that would help. Nothing here is built. The set works on the stock binary without any of it.

Ordered by what it would buy.

## 1. Footfall and animation events

Today: a script learns nothing about the animation. `kj\anim\footfalls.sqs` works the footfalls out from the outside: while he walks the engine moves him by step x phase, so distance walked is the phase; in a turn and in the stomp it counts seconds from the move's start. That is good to about a quarter of a second on the first step after a start and exact afterwards, but it cannot see a blend, a rate change inside a playMove, or a foot the crush feature wants at the frame it lands.

Engine: the footfall moment already exists. `Man::AdvanceMoveQueue` tests the move's two sound edges every frame and, when the phase crosses one, sets `_doSoundStep` and drops a footprint mark at the `stopaL` / `stopaP` memory point (`World/Entities/Infantry/SoldierOldMove.cpp:186-226`; the points are looked up at `SoldierOldSimProxy.cpp:1439-1440`). It only runs when the move has `soundEnabled = 1`.

Wish, smallest first:
- At that spot, raise a script event `"Footfall"` with `[unit, isLeft, worldPosOfTheFootPoint]` (and call the native crush service with the same). `moves_dedushka.hpp` already carries the walk's edges (`soundEdge1`, `soundEdge2` from the generator); the turn clips and the stomp would get theirs the same way.
- Better: a per-move list `events[] = {phase, "name", ...}` raising `"AnimEvent"` `[unit, move, name]`. The generator already writes every event with its phase (`ded_anim_timing.json`): roar start, stomp impact, the swat's sweep, ribs open.
- Let the edge test also run for the secondary move while two moves blend, or footfalls are lost during a blend.

## 2. Read an animated point from a script

Today: no script command returns where a memory point or a selection is (`getPos` only). The beam's origin (the furnace), the swatting hand and the feet are published as numbers in `ded_anim_timing.sqf`, posed offline for fixed moments. Between those moments a script has to guess, and when the body is blending or turning the numbers are off.

Engine: the code is there for the footprint (`AnimatePoint(_shape->FindMemoryLevel(), index)` then `PositionModelToWorld`, `SoldierOldMove.cpp:210-211`).

Wish: `unit selectionPosition "name"` (animated, model space) and `unit modelToWorld [x, y, z]`, or one command returning the world position of a named memory point. The model then carries `ded_furnace`, `ded_furnace_dir`, `ded_mouth`, `granat`, `granat2`, `stopaL`, `stopaP` (SKELETON.md, section 4) and every feature reads them live.

## 3. Feet that stay planted when the engine moves the body in ways the clip cannot know

Measured or computed for the proxy; see the anim NOTES.md for the numbers.

- **Starts and stops.** During a blend the engine advances the body by each move's step weighted by the blend factor (`SoldierOldMove.cpp:181-184` for the primary, `269-272` for the secondary) while each clip's own phase runs at full rate. So the planted foot of the walk slides by about speed x blend time / 2 on every start (about 1.1 m at 2.3 m/s and a 1 s blend), and on a stop both feet are dragged together from wherever they were (up to 5 m each). A start clip or a stop clip cannot fix it, because the slide is in the blend itself.
- **Turning while walking.** The body turns in any move, capped by the `turnSpeed` of the current move's action class (`SoldierOldSim.cpp:77-94`). A planted foot 5 to 8 m from his centre is swung round with the body. The set limits the damage with a low cap while walking (`DED_TURN_WALK`, 0.05 rad/s) and gives the real turning to the turn clips, which step round at exactly the cap (`DED_TURN_SPOT`). When the AI turns slower than the cap (the last few degrees), the turn clips over-step.
- **Slopes.** `onLandBeg = 0` keeps him upright and `legs = "legsNo"` turns the slope skew off (it is tuned for a 0.4 m stance). On a slope one foot floats and the other sinks: across a 10 m stance, a 10 percent grade is a metre.

Wish: a foot lock. While a foot's sound edge says it is down, hold its world position (and the ground height under it) and solve the leg, two bones, after the clip is posed. It needs the foot, shin and thigh bone indices and the `stopa` points, all of which the type already has. That one change fixes starts, stops, walking turns and slopes. Cheaper partial wishes:
- weight the step of both moves by phase rate, not blend factor, when the two moves share an `equivalentTo` group, and let a move declare `interpolationRestart` only for edges from outside its group (today the Low and Crouch copies of the walk restart the cycle when the AI changes stance mid stride, `SoldierOldMove.cpp:338-349`);
- a per-type "turn only on the spot" flag: no body turn while the primary move has a step, and a configurable threshold for the stop-and-turn rule (now fixed: no speed above 45 degrees of heading error, `World/Entities/Infantry/SoldierOldAI.cpp:1786-1789`; turn moves only when stopped and `_turnWanted` above 0.5, `2223-2227`).

## 4. Gait and steering under the AI

Known from the chupacabra: `doMove` puts the unit in a new subgroup whose speed mode is always NORMAL (`AI/AIGroupCmd.cpp:223-249`), so `setSpeedMode` never reaches it. Dedushka has one gait, so this bites less, but the brain cannot ask for the slow walk (the walk's band is 0.6 to 1 of full rate; only the AI's own slowing near a path end uses it).

Wish: copy the main subgroup's speed mode into the new subgroup, or a command that sets a unit's wanted speed directly; and a native straight-line steer (the survey's item 12) so he can walk through a town instead of threading its streets.

## 5. A partial-body or additive layer

Today a move owns the whole body. `DedFlinch` therefore stops the walk: a playMove from `DedWalk` blends the legs to the stand. The brain is told to flinch only when he stands; when he walks, hits do not show. The same limit means no breathing on top of the walk and no head turned toward a target (`head = "headNo"`).

Engine: `BlendAnims` already mixes per-bone factors for aiming, legs and head (`MoveInfo` reads `aiming`, `legs`, `head`, `SoldierOldSimProxy.cpp:1234-1239`).

Wish: let a second move play over a bone mask (spine, neck, head, arms) with its own time, added to whatever the legs are doing. `playMove` with a mask name would do.

## 6. Held moves and playMove

Today a `playMove` of a loop lasts exactly one cycle: the external move is "finished" the first time the primary wraps (`SoldierOldMove.cpp:163-178`, `291-304`), then the AI takes over. To hold the beam pose and the dormant pose for as long as a script wants, their action classes map every action to the state itself (the offline checker calls that a stance loop; it is on purpose). It works, tested, but it is a trick, and a script cannot cancel a queued or running playMove except with `switchMove`, which pops.

Wish: `unit playMove [name, seconds]` (hold a loop that long) and a command to drop the external queue with a blend.

## 7. What is playing

`getMove` returns the END of the move queue, the move he is heading for, not the one on screen (`Man::GetCurrentMove`, `SoldierOldMove.cpp:408-413`). The tests take their timings from the `AnimChanged` event instead.

Wish: a command returning `[primary move, its time 0..1, blend factor, secondary move, its time]`. With it a script could place a footfall or a hit exactly, even without item 1.

## 8. Key blending and the animation cache

The engine blends the two keys round a time element by element (`AnimationRT::PrepareMatrices`, `World/Simulation/Animation/RtAnimation.cpp:1036`), which shrinks a bone that turns far between two keys, so fast parts need dense keys (the stomp has 112, the generator fails a clip over 15 degrees per key). The cache holds 32000 matrices for all loaded clips (`MaxAnimationRTMatrixCount`, `RtAnimation.cpp:366`); this set is 23661 with 33 bones, so a model with many more bones, or a few more clips, starts evicting (a hitch, not a failure).

Wish: raise the cache limit (it is one constant), or blend rotation and position separately.

## 9. Step sound: usable today, two small wishes

The engine can already play his footfall at the right frame: set `DED_STEP_SOUND 1` in `moves_dedushka.hpp` and give the walk a `soundOverride = "dedstep"` naming an entry of the creature's `SoundEnvironExt` class (`Man::Sound`, `World/Entities/Infantry/SoldierOldSim.cpp:1462-1500`; the entry is found by `GetEnvSoundExtRandom`, line 1474). That is for the roar feature to decide; it is off now because the stock entry is a man's step.

Wish: play it at the foot's memory point, not under the body's origin (`land = Position()` at `SoldierOldSim.cpp:1491`), and let the volume be scaled per type.

## 10. A turn that is part of the clip

Today the clip cannot turn the body: only the engine's `turn` does, at a rate the clip must guess. If an RTM could carry a yaw per cycle (as it carries a step), a turn on the spot would be exact at any rate, like the walk.
