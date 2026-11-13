# ROAR: what the engine agent could add

Written by the roar feature agent, 2026-11-11. Paths are relative to `arma_CWR/engine/Poseidon` unless they
start with `../`. Every line cited was read in the kaiju worktree (origin/main `3df39c30`); nothing here was
built. The roar scripts run on the stock binary today; each item below says what the script does instead
and what would be better. Most useful first.

## 1. Camera tremor (the largest gap)

- Today: the engine has no camera shake. The view transform is built in `World/World.cpp:1201-1235` from the
  camera vehicle's own transform and nothing offsets it.
- Script stand-in (`kj\roar\shake.sqs`): the player's body is moved a few centimetres and a fraction of a
  degree in heading every frame. It works on foot in first person. It cannot roll or pitch the view, it does
  nothing in aircraft or boats, in a ground vehicle it is one upward kick, and it moves the player for real
  (a few centimetres, undone at the end).
- Wish: a tremor source with a position, a strength, a full-strength radius, a zero radius, a duration and a
  rate, summed into a camera offset (position and a little roll and pitch) for whatever camera is active:
  first person, third person, a vehicle, optics, a cutscene camera. The engine work in `Game/Kaiju` is
  already adding `kjShake [pos, strength, fullRadius, zeroRadius, seconds, rate]` and `kjShakeGain`.
  `shake.sqs` calls `kjShake` when the mission sets `KJ_native = true`, with the same position, radii and
  duration the stand-in uses; the stand-in stays as the fallback.
- Also wanted: a way for a script to ask whether the kaiju commands exist. A script cannot test for a
  command, and naming an unknown one is a script error, so today the mission has to say `KJ_native = true`.
  A config entry would do (for example `CfgKaiju >> version` in the base config the binary ships with).

## 2. Sound that arrives late

- Today: the speed of sound (330 m/s, `Audio/SoundScene.cpp:368-382`) is applied to shots, explosions and
  collapses only (`AI/VehicleAICombat.cpp:682, 727, 866, 2328`; `Graphics/Rendering/Effects/Smokes.cpp:294,
  409`). Script `say` starts at once (`Audio/DynSound.cpp:84-125`).
- Script stand-in (`kj\roar\snd.sqs`): waits distance / 343 s, then says the sound from a helper object. The
  wait is fixed when the sound is sent; a listener who moves during it is not followed.
- Wish: a flag on `say` (or `kjSound [object, class, point]`) that calls `SimulateSpeedOfSound` on the wave,
  as the gunshot path does.

## 3. A looping sound on him

- Today: `SoundObject` can loop (`Audio/DynSound.cpp:186`, the `looped` argument) but `say` always passes
  false (`Game/Commands/GameStateExtUi.cpp:1050`), and there is no `createSoundSource`.
- Script stand-in (`kj\roar\presence.sqs`): the furnace drone is a seamless 8 s file said again every 7.9 s
  from a helper object that is moved to his chest once a second.
- Wish: a looping 3D sound bound to a memory point of the unit, started and stopped by script, with the
  volume settable while it plays (the furnace should swell before a beam and dim after it).

## 4. A unit that speaks queues its own sounds

- Today: a `say` on a unit with a brain waits until that unit has finished its last `say` and its radio
  (`Audio/DynSound.cpp:197-209, 221-228`). A roar said by Dedushka himself would wait behind his furnace
  drone or his last footfall.
- Script stand-in: nothing is ever said by him. Seven invisible `HeliHEmpty` objects are the sound sources
  (`KJR_fnEmitter` in `kj\roar\init.sqf`), moved to his mouth, chest and feet as needed.
- Wish: with item 3 this goes away. Otherwise a way to `say` on a unit without the queue.

## 5. A footfall event

- Today: each move has two sound edges; crossing one sets `_doSoundStep`
  (`World/Entities/Infantry/SoldierOldMove.cpp:186-201`) and the engine plays the type's step sound
  (`World/Entities/Infantry/SoldierOldSim.cpp:1476-1509`). Script is not told.
- Script stand-in: `kj\crush\loop.sqs` (and `kj\roar\walk.sqs` when crush is not running) counts the ground
  he has covered and calls `step.sqs` every 0.22 heights. It drifts against the animation: the dust and the
  thump are not tied to the frame the foot lands.
- Wish: `kjSteps unit` is being added (count, which foot, position). Better still an event handler
  (`"Footfall"`, passing the foot position from the foot's memory point) so nothing has to poll.

## 6. Fear

What the roar does today is in `kj\roar\panic.sqs`; it leans on the engine's own group flee
(`AI/AIGroup.cpp:1259-1276`, `AI/AIArcadeActions.inc:1815-1976`). Four things in the engine get in the way:

- **Script-made groups never flee.** `AIGroup::_maxStrength` starts at 0 (`AI/AIGroup.cpp:74`) and is only
  computed for groups loaded from a mission (`AI/AICenterImpl.cpp:2058`), after a join
  (`AI/AIArcade.cpp:1900`) or when a driver gets in or out (`AI/AIUnitImpl.cpp:584, 730, 860`).
  `CreateUnit` (`Game/Commands/GameStateExtWorld.cpp:155-235`) does not call `CalculateMaximalStrength`, so
  for a group made by `createGroup` and `createUnit` the test `strength < (1 - courage) * 0` is never true.
  Wish: call `CalculateMaximalStrength()` after `group->AddUnit(unit)` there. Guerrilla Mode's own spawns
  should be checked for the same hole.
- **A script-made group's flee point is the map corner.** `GroupCreate` gives the group a first waypoint at
  `VZero` (`Game/Commands/GameStateExtWorldConfig.cpp:703`) and `FindFleePoint` falls back to waypoint 0
  (`AI/AIArcadeActions.inc:1915-1933`). `panic.sqs` moves waypoint 0 before it frightens anyone.
- **The only script handle on strength is damage.** `allowFleeing 1` sets courage to 0, but a group at full
  strength still does not flee (`strength < maxStrength` is false). `panic.sqs` gives one unhurt man 3
  percent damage for the length of the panic and takes it back after. Wish: a fear source in the engine:
  `kjFear [pos, breakRadius, seconds]` that calls `AIGroup::Flee` (`AI/AIGroupImplHealth.cpp:1587-1605`)
  for every non-player group with a man inside the radius, with the flee point set away from the position,
  and `Unflee` when the time is up. No damage, no waypoint edit, and it would work for any group.
- **There is no fear of size.** Fleeing is strength based only; nothing looks at what the enemy is. A real
  version would add the creature's presence to the group's strength test.

Seen in runs, same area:

- Under the engine's flee an unarmed civilian group walked (6 km/h, move `civilwalkf`) where a plain
  `doMove` at full speed ran (12 to 16 km/h, `civilrunf`). Not traced. The roar moves civilians by script.
- A man held with `stop true` does not turn his body for `doWatch` (180 degrees off after eight seconds).
  `freeze.sqs` and `turn.sqs` swing the body round with `setDir`. A man who is not stopped drifts back
  toward his formation place afterwards (the second man of a pair ended 67 to 111 degrees off).
- The garrison the engine's own Guerrilla service spawned in Morton could not be made to flee by the
  engine's rule either (the run was scripted in every test run): the hole in the first bullet is not
  only a script problem.

Smaller, same area:

- `AIUnit::SetDanger` (`AI/AIUnitImpl.cpp:2200-2223`) and `Man::Scream`
  (`World/Entities/Infantry/SoldierOldSim.cpp:1571-1596`) cannot be reached from script. A frozen soldier is
  `stop true` plus `doWatch`; he does not go to danger mode and does not cry out.
- AI does not perceive anything past `TACTICAL_VISIBILITY` (900 m, `Core/EngineState.hpp:13`), so the roar
  `reveal`s him to every group in earshot by script.
- There is no courage getter. After a panic the script restores `allowFleeing (1 - skill leader)`, which is
  the engine's own default (`AI/AIGroupImplHealth.cpp:1568-1580`), but a mission's own `allowFleeing` value
  is lost.

## 7. Street lamps: a crash, and they cannot be searched for

- **Bug, seen in a run.** `lightIsOn` and `switchLight` accept any entity as a street lamp. `StreetLamp`
  has no casting id of its own (`World/Scene/ObjectClasses.hpp:88-118` has no `USE_CASTING(base)`; compare
  `Road` at `:165`), so it shares the id of its base class and `dyn_cast<StreetLamp>`
  (`World/Scene/Object.hpp:819-829`) succeeds for every entity that derives from that base. The comment at
  `World/Scene/Object.hpp:90-93` describes exactly this failure. Observed: `lightIsOn` of a `SoldierEB`
  answered "OFF" instead of "ERROR", `switchLight "OFF"` on him wrote into the soldier, and the game died
  one frame later with an access violation in `EntityAI::GetHit` from `Man::IsAbleToStand`
  (`kaiju-notes\features\roar\evidence\streetlamp-cast-crash.txt`). Any mission script that follows the
  usual idiom (ask every nearby object `lightIsOn`, switch the ones that do not answer "ERROR") can kill the
  game. Fix: `USE_CASTING(base)` in `StreetLamp` and a matching `DEFINE_CASTING(StreetLamp)`. Until then the
  roar scripts recognise a lamp by its class name only (`KJR_fnIsLamp`: `typeOf` starts with "StreetLamp").
- Today: `nearestObjects` with a type filter only returns AI entities
  (`Game/Commands/GameStateExtUi.cpp:633-640`); a street lamp is an `EntityHit`
  (`World/Scene/ObjectClasses.hpp:88`). So `presence.sqs` takes every object in a circle and asks each one
  for its class (`typeOf`). In a town that is hundreds to thousands of objects per scan; the script rescans
  only after he has moved three heights.
- Wish: let the type filter match any entity type (or add `nearestLamps [pos, radius]`). Also wanted: a way
  to dim or flicker a lamp (today it is on, off or auto, `World/Scene/ObjectClasses.cpp:33-38`), and house
  windows that can go dark with the street.

## 8. Animals

- Today: the only animal is the seagull, a camera-class object with no brain; `World::InsertSeaGulls()` is
  an empty stub (`World/WorldInit.cpp:307`). `birds.sqs` makes its own with `camCreate` and flies them with
  `camSetPos` and `camCommit`. There are no dogs, so "the dogs start barking" is not possible without a
  model and a sound.
- Wish (low priority): an ambient flock service that takes a fear position and sends birds up around the
  camera, away from it.

## 9. Memory points from script

- Today: no command reads a selection or memory point position. The roar comes from 0.85 of his height above
  his origin, the furnace from 0.6, a foot from fixed fractions of his height (`KJR_MOUTH_H`, `KJR_CHEST_H`,
  `KJR_STANCE`, `KJR_FOOT_AHEAD`). When he bends, the sound source does not follow.
- Wish: `kjPoint [unit, "name"]` returning the world position of a memory point in the current pose, and a
  config value `kjHeight` on the class so nothing is guessed (the scripts already read it when it is there).

## 10. Quiet runs cannot check that a sound file loads

- Today: with `--nosound` the dummy audio system reports 1.0 s for any file name and never opens the file
  (`Audio/Dummy/SoundSystemDummy.cpp:38-61`). A test can prove the class resolves (`soundLength` is 0 for an
  unknown class) and the build checks every file with the engine's loader (`PoseidonTools sound inspect`),
  but nothing proves the game finds the file inside the pbo.
- Wish: let the dummy system open the file for its real length (`SoundLoadFile`, as
  `../PoseidonOpenAL/SoundSystemOAL.cpp:256` does), so a missing or broken file fails a quiet test.

## 11. One sound, near and far

- Today: the scripts pick a near or a far file by the listener's distance when the sound is sent. There is
  no cross-fade and nothing changes while it plays.
- Wish: distance layers on a sound class (near file, far file, cross-fade distances) and air absorption by
  distance (the EFX low-pass is already in the OpenAL backend for environments).
