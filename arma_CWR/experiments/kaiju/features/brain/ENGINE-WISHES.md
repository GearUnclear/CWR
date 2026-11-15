# Brain: what the engine could add

Written by the brain feature for the engine agent. Each item says what the brain has to do today on the
stock binary, what would be better, and where the limit sits in the source. Paths are relative to
`arma_CWR/engine/Poseidon`. The engine agent's `features\engine\API.md` already answers some of these
(marked "API has it"); the brain does not use any of it yet, so everything below still runs on the stock
binary, and each wish names the fallback that would stay.

Most useful first.

## 1. A walk whose pace the script sets

- Today: the brain walks him with `playMove`. While a script move plays, the AI pilot sets the wanted
  speed to zero (`World/Entities/Infantry/SoldierOldAI.cpp`, `Soldier::AIPilot`, the external-move
  branch) and the clip is then played at the move's `relSpeedMin` (`SoldierOldSim.cpp`,
  `Man::SimulateAnimations`, `adjustTimeCoef = _walkSpeedWanted / speedZ` clamped to the move's range).
  So the pace of a scripted walk is one config number and cannot change at run time. Measured: `DedWalk`
  (`relSpeedMin` 0.6) goes 1.38 m/s under `playMove`; the same clip with `relSpeedMin` 1 goes 2.3 m/s.
  The brain's build adds that as a second move, `DedStride`, to its own copy of the addon.
- Wish: `kjPace [unit, rate]`, a per-unit multiplier on the playback rate of whatever move is playing
  (0.5 to 1.5 is plenty). Then one walk clip gives a trudge, a walk and a hurried walk, and he can slow
  into a stop and lean into a start instead of switching speed at a stride boundary.
- API has it, partly: `kjSteer` takes "walk", "slow" or "fast", which picks a different action of the
  moves class. That needs three clips or three states; a rate needs none.

## 2. Stop, or change move, in the middle of a stride, with a blend

- Today: a `playMove` clip cannot be left before it has played once. `switchMove` leaves at once but
  snaps the pose, and it puts the interrupted move back on the queue so it plays again afterwards
  (`SoldierOldMove.cpp`, `Man::SwitchMove`: "return _externMove (if any) to queue"). There is no script
  way to empty that queue. Result: every reaction of his waits for the end of the stride (up to 7 s),
  and a hit while he walks cannot stagger him.
- Wish: `kjPlayNow [unit, move]`: drop the queue and blend from the current pose into the move through
  the move graph (what `SetMoveQueue` does for the AI), and `kjQueueClear unit`.
- With it: a real flinch on impact, a stop that plants the lifted foot, and the wake out of the first
  dormant cycle without waiting for the clip to wrap.

## 3. Reading the move, its phase and the queue

- Today: `getMove` is all there is. The brain works out where he is in a stride from the ground he has
  covered, and it cannot tell whether a move it queued has started. It cannot ask whether the addon has
  a move at all: it makes a second body far away for a fifth of a second, tries each move on it with
  `switchMove`, and deletes it (`kj\brain\start.sqs`).
- Wish: `kjMove unit` returning `[move, phase 0..1, moves still queued]` and `kjHasMove [unit, name]`.
- API has it, partly: `kjSteps unit` gives the footfall count and position, which covers the footfall
  timing but not the phase.

## 4. He must not be shoved by men and cars

- Today: an AI man is pushed out of every man and vehicle he overlaps, by the full overlap, whatever the
  masses (`SoldierOldSim.cpp`, the object-collision block after `onlyVehicles`; read by the AI survey,
  section 6). A soldier standing in his way pushes a 20 000 tonne body aside. The brain's walk is a
  straight line only on open ground; in a street with people and parked cars he would be nudged off it
  and steer back.
- Seen, probably: in one of the two demo runs with every feature in, something moved him about 8 m in
  one second (top speed 7.98 m/s against his 2.31) and he ended 4.7 m off his usual stopping point; no
  script moves him. The second run showed no jump. Not traced; `kj\brain\watch.sqs` now records every
  jump (`KJT_jumps`) for the next time.
- Wish: a trait that skips the push-out for him against anything lighter than a set mass (and applies
  the crush instead).

## 5. Being counted as dangerous, and fear

- Today: nobody reacts to him. He has no weapons, so groups do not count him as a dangerous enemy: no
  contact call, no combat behaviour, no dismount, and the exposure map has nothing at his feet
  (`AI/AIGroupImpl.cpp:605-611`, `AI/AICenterImplPreview.cpp:271`). Nothing in the engine flees from a
  thing; a group flees only when its own strength has dropped, and then to its first waypoint
  (`AI/AIGroup.cpp:1259-1276`, `AI/AIArcadeActions.inc:1815-1924`). The roar feature fakes panic by
  script.
- Wish, in order: (a) a trait that makes groups count him as a dangerous enemy and paints his
  surroundings on the exposure map; (b) flight away from a named threat instead of to the first
  waypoint; (c) a fear radius that lowers courage near him. The AI survey lists thirteen such items
  with sizes: `kaiju-notes\features\brain\ai-engagement-survey.md`, "Engine changes that real fear
  would need".

## 6. Knowing what hit him and from where

- Today: the stock `Hit` event needs a single hit worth 0.05 of total damage, which no stock weapon
  reaches on him. The crush feature reads the rise in `getDammage` and guesses the shooter from the last
  heavy `Fired` it heard; the brain takes its signal (`KJ_hurt`). Shots are heard through a `Fired`
  handler the brain puts on every unit within earshot, renewed every 8 s. Explosions and impacts that
  nobody "fired" (satchels, bombs already in the air, script damage) are not heard at all.
- API has it: `hitReport` and `kjHits`. The brain should read `kjHits` when it is there (position and
  attacker of every hit) and keep the handler route for the stock binary.
- Wish on top: one world event for loud things, "something exploded or fired here, this loud", so that
  noise is a real sense and not a handler on every unit.

## 7. Ground and water under a point

- Today: there is no script reader for terrain height or water depth. The brain keeps one invisible
  helipad object, moves it to the point and reads its height above sea level; over the sea that reads
  the sea surface, so "dry" is "higher than the sea", and he will not step onto any ground below about
  3 m above sea level, shallow or not.
- Wish: `kjGround [x, y]` returning `[ground height above sea level, water depth]`.
- API has it, partly: the `wadeDepth` trait lets him stand in water. With it the brain's sea guard
  should compare the depth with his wade depth instead of refusing all water.

## 8. Lights as something to read

- Today: `lightIsOn` answers for street lamps only, and it must be asked only of a street lamp (asked of
  a soldier the engine answers as if he were one; the roar notes have the crash that follows a
  `switchLight` on a man). Headlights, searchlights, fires lit by script and burning wrecks cannot be
  read. The brain's "lights draw him" is street lamps only.
- Wish: `kjLights [position, radius]` returning the lit things there with a brightness each (lamps,
  vehicle lights that are on, fires, burning wrecks).

## 9. Small evaluator gaps that cost a workaround each

- No `typeName`: the brain tells an array from an object by formatting it and looking at the first
  character.
- No way to ask whether a file is in the mission: `loadFile` of a missing file returns "" but the engine
  logs "Script ... not found" as a warning. The brain reads a list written by its run script instead
  (`kj\brain\have.sqf`).
- No reader for config values outside `CfgVehicles` (`GetVehicleParam` exists; a moves class cannot be
  read), so clip lengths come from a generated table or from numbers in the script.
- Global names are case-insensitive. `KJB_VISITED` (a tuning number) and `KJB_visited` (a list) were
  one variable, and the first test run died on it. A warning when a script assigns an array to a name
  that holds a number, or a lint for names that differ only in case, would have caught it.

## 10. The harness: a failed assertion must not hold the machine

In run `f1-full-1111-210417` the test failed one `triAssert`. The game then stopped answering the
harness but kept running, and tri kept waiting, so the machine's one game slot was held until the game
was stopped by hand seven minutes later (tri then reported "broken pipe" on that assertion). In an
earlier run a script error under `--autotest` ended the game at once, which is the behaviour wanted
here too: on a failed assertion, answer, and exit.

## 11. For the animation feature, not the engine: one more move

`class DedStride : DedWalk { relSpeedMin = 1; }`, with `DedStride` added to the `dedUp` interpolation
group. It is the walk at full rate under `playMove` (item 1 says why). The brain's
`build-kj-brain.ps1` adds it to its own copy of the addon today; with it in `moves_dedushka.hpp` that
copy is no longer needed. If item 1 (`kjPace`) lands, neither is.

## 12. Path planning: nothing to add for the brain

The brain does not use the planner, so the `obstacleBlind` trait is not needed for his walk. It stays
useful if some other unit (or a later, simpler mode of his) is moved by `doMove`.
