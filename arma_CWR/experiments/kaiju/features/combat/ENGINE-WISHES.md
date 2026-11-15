# Combat: what the engine could do better (2026-11-12)

For the engine agent. Nothing here is needed for the feature to work as built; each item removes a
fake or a guess. Paths are under `arma_CWR\engine\Poseidon`. Evidence is in
`D:\Arma_CWA_worktrees\kaiju-notes\features\combat\NOTES.md`.

In order of what it would buy.

## 1. Say in the hit report whether it was the strike or the blast

- File: `World\Entities\Weapons\Dammage.cpp`, `Object::DoDammage`, the hook
  `Kaiju::OnLocalDammage(this, owner, ammo, pos, delta)`; `Game\Kaiju\KaijuService.cpp`
  `OnLocalDammage` and `HitRecord`; `Game\Kaiju\KaijuCommands.cpp` `Hits`.
- What: pass `valRange` (negative for a direct hit, positive for a blast) into the hook, keep it in
  the record, and add it to each `kjHits` entry as a seventh element (`true` for a strike).
- Why: one round that strikes him is reported twice in the same instant at the same place (the
  strike from `DirectDammage`, then its own blast from the `ExplosionDammage` loop), and a blast
  beside him once. A strike high on his body can lose its blast report, because the loop skips an
  object whose origin (his feet) is farther than four blast ranges plus his radius from the impact
  (`World\Simulation\Collisions.cpp`, `Landscape::ExplosionDammage`, `dist2 > maxDist2`). The script
  pairs the two reports by time, ammo and place, and tells a lone strike from a lone blast by the
  size of the engine's damage number, or for a round too small to move that number by whether the
  point lies on his body. That is a guess the engine can make unnecessary with one float.

## 2. Let the creature say what the AI may fire at it, apart from its armour

- File: `World\Detection\Target.cpp`, `EntityAI::WhatShootResult`, the two armour gates
  (`relStrength = hit * targetInvArmor < 0.1` and
  `changeDammage = hit^2 * targetInvArmor * (0.27 / tgtRadius)^2 < 0.004`).
- What: a trait `aiMinHit` (and optionally `aiMaxHit`): for a unit that has it, replace both gates
  with `ammo.hit >= aiMinHit`.
- Why: today "who shoots" is set through the type's `armor` and the model's geometry radius
  together, so the cut moves when the model changes (the proxy's radius is 19.7 m; a final model
  with a longer body moves every threshold). The same number also feeds the AI's estimate of how
  fast it is killing him, which decides WHICH weapon it picks: with the 30 mm cannon let through the
  gates, a Mi-24 strafed with the cannon and fired one missile per pass, and never its rockets
  (NOTES.md, "Aircraft"). A clean threshold, plus the surplus ranking seeing his real toughness,
  would let autocannon join in without gunships preferring it.

## 3. A way to dim and darken the furnace

- File: the model export (`experiments\kaiju\addon\export_dedushka_p3d.py` drops every vertex group
  that is not a bone, so no named selection reaches the p3d) and `AI\VehicleAICombat.cpp`
  (`EntityAI::Animate`, the `_hiddenSelections` block: it already swaps a texture on a named
  selection for any EntityAI, a Man included, through `setObjectTexture`).
- What: either keep a named selection `kj_furnace` over the glowing faces in every visual LOD and
  list it in `hiddenSelections[]` (then scripts can swap in a dimmer or a dead texture; note that a
  hidden selection with no texture set is hidden, so the script must set one at once), or a native
  `kjTint [unit, "selection", [r, g, b]]` that scales the colour of fullbright faces.
- Why: the furnace is a fullbright texture. The engine cannot dim it, so "the furnace flickers and
  dims" is done with smoke hung in front of it and a small light at night, and the dead body's
  chest still shows its painted fire where the body arches off the ground (the notes' `shots` 15 and
  22). The real models, whose furnace shows from the side, will lie there with the fire painted on.

## 4. Count him as a dangerous enemy

- File: `AI\AIGroupImpl.cpp`, `AIGroup::CreateTargetList` (the count of `_enemiesDetected` only for
  hostile targets whose type threat is above zero) and `AI\AICenterImplPreview.cpp` (the exposure
  map skips a type with no weapons).
- What: a trait `menace` (0 or 1): a hostile unit that has it counts as a detected dangerous enemy
  and paints the exposure map around itself as a heavy weapon would.
- Why: measured in every live-fire run here: with forty metres of monster under fire two hundred
  metres away, the infantry section stayed in AWARE, upright, in formation. Nobody calls contact,
  goes to ground or dismounts, because he carries no weapon. The fight looks one sided in the wrong
  way: the men do not behave as if anything were happening.

## 5. Keep aircraft off his head, and let gunships stand off

- File: `World\Entities\Vehicles\Air\HelicopterAI.cpp` (the sweep height: aim point height plus 30
  to 50 m) and `Air\AirplaneAI.cpp` (collision avoidance skips anything not airborne).
- What: for a target with a kaiju trait, a minimum pass height of its own height plus a margin, and
  a stand-off distance for guided weapons (fire from range instead of flying the sweep through him).
- Why: measured: a Mi-24 flew its attack runs straight over him, 56 m above his feet, 16 m over his
  head, closest 59 m from his origin. That is inside the reach of the swat. Good for him, but it is
  not what a pilot would do, and it makes gunships cheap.

## 6. The hit report queue as an event, and a bigger queue

- File: `Game\Kaiju\KaijuService.cpp` (`kMaxHits` is 64, `OnLocalDammage` drops the oldest).
- What: an event handler (`"KaijuHit"`) or a queue of a few hundred.
- Why: a rifle section and two machine guns make more than 64 reports in a tenth of a second on a
  slow frame; the dropped ones are only bullets today, but a dropped tank round would be a lost
  wound. The script reads the queue ten times a second.

## 7. Scripted damage that carries an ammo class

- File: `Game\Kaiju\KaijuCommands.cpp`, `Damage` (`kjDamage`).
- What: an optional sixth element, an ammo class name, stored in the report.
- Why: a mission's own ordnance (a naval gun, a scripted bomb) reaches the health model with an
  empty ammo name and is weighed by the engine's damage number alone. With a class it would be
  weighed like the real round. Today the script offers `KJCB_fnScriptHit` for that instead.

## 8. A pace the script can set (the brain's wish, repeated because wounds need it)

- What: a per-unit animation rate for the walk (the brain's ENGINE-WISHES.md, item 1).
- Why: "he slows as he is hurt" is published as `KJ_woundPace` (1 down to 0.45) and nothing can
  apply it: a script move plays at one rate. Today the brain only lengthens his pauses.

## 9. A flinch that does not own the whole body (the anim feature's wish, repeated)

- What: an upper-body layer for `DedFlinch`.
- Why: a flinch is a played move, so it cannot be played while he walks. Under steady fire a
  walking giant never flinches; a standing one flinches every few seconds.

## 10. He is shoved while he is being hit

- File: `World\Entities\Infantry\SoldierOldSim.cpp`, the object-collision block of `Man::Simulate`
  (an AI Man is moved out of every non-static object that overlaps him, by the depth of the
  overlap, with no mass term).
- What: skip that push-out for a unit with a kaiju trait, at least against shots, men and light
  vehicles (the brain's and the crush feature's wish, extended to projectiles).
- Why: measured in `combat_heavy`: standing still, told not to move, heading held, he ended 7 to
  14 m from where he stood after about 110 rockets and 115 tank rounds. The cause was not traced:
  no vehicle or man came near him, and a shot is an object too, so the push-out is the suspect. A
  forty thousand tonne animal should not slide under rocket fire.

## 11. A crash on the way out when a light is still bound to a unit (a bug, with its stack)

- File: `Game\Kaiju\KaijuService.cpp`, the static `s_units` and `Unit::~Unit`, `KaijuLight::~KaijuLight`.
- What happens: when the game exits while a `kjLight` is still on a unit, the process dies with an
  access violation (exit code -1073741819) after "Shutdown complete". The stack, the same in three
  runs of three (`kaiju-notes\features\combat\runs\exit-1112-033733\r1_combat_probe_exit\game_stderr.log`):
  `dynamic atexit destructor for 's_units'` -> `Kaiju::Unit::~Unit` -> `Kaiju::KaijuLight::~KaijuLight`
  -> `AttachedOnVehicle::~AttachedOnVehicle` -> `World::RemoveAttachment` -> `FindArray<...>::Find`.
  The static list of kaiju units is destroyed by the C runtime at exit, after the world it detaches
  its lights from is gone.
- What: empty `s_units` (or at least drop every light and emitter) when the mission ends and before
  the world is torn down, so nothing is left for the static destructor to do.
- Why: any test or play session that ends with a light on him crashes at exit and tri counts the run
  as failed, even with every check passed (it cost `combat_wounds` one gate run). The integrated
  set's furnace light is on for his whole life, so it is exposed unless it takes the light off first.
  This feature's tests now let its loops end (which removes its light) before they exit;
  `probes\combat_probe_exit` reproduces the crash on purpose.

## 12. Small things

- `Hide body`: a soldier whose type has `canHideBodies` gets the action on any dead Man
  (`World\Entities\Infantry\SoldierOldActions.cpp`, the `ATHideBody` block) and the engine then
  sinks and deletes the body (`SoldierOldSim.cpp`, `_hideBody`). A forty metre corpse can be hidden
  by one Black Op. Skip it for a unit with a kaiju trait.
- Side: observers judge him by his config side until they have confirmed it (about 180 m for
  infantry). The combat addon variant makes his config side Resistance for that reason. A trait
  `sideKnown` (always use the instance side) would let a mission put him on any side.
