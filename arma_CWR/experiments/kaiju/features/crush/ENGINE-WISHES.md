# CRUSH: what native help would improve it most

Written by the crush feature agent, 2026-11-11. The script feature in `kj\crush\` works on the stock
binary; every item below replaces a guess, a fake or a workaround in it. Paths are relative to
`arma_CWR\engine\Poseidon`. "Measured" means seen in a game run (logs under
`kaiju-notes\features\crush\runs`); the rest is read from the source. Ordered by how much each one
would improve what the player sees.

## 0. One crash to fix first: `getMass` on an object with no model

Measured (probe 1, `runs\probe1\zz_crush_probe\game_stdout.log`, 20:12:44): `getMass` on the engine's
own collapse helper crashes the game (access violation in `Object::GetMass`). `World/Scene/Object.hpp:456`
is `return _shape->Mass();` with no null check, and `ObjectDestructed` is built with a null shape
(`Graphics/Rendering/Effects/Smokes.cpp:399-400`). One such helper exists for 2 to 3 s at every felled
building and tree, and an unfiltered `nearestObjects` returns it. The crush script works round it
(`KJC_fnKind` only asks the mass of things it knows have a model). The fix is a null check in
`Object::GetMass` / `GetInvMass`, or in `ObjGetMass` (`Game/Commands/GameStateExtGrp.cpp:373`).

## 0b. A church never shows its collapse

Measured (`runs\gate3x3-1111-212107\r3_crush_walk`, shots `009_w09` to `011_w11`): he walked straight
through Morton's church. `setDamage 1` destroyed it as far as the engine's state goes (damage 1, the
collapse sound and dust), but the building stood untouched around his legs. `Church::Animate`
(`World/Entities/Vehicles/House.cpp:1014-1037`) turns the clock hands and never calls the base
`Animate`, which is where a destroyed object is morphed to its collapsed shape
(`World/Scene/Object.cpp:318-345`). `Fountain::Animate` calls it (`House.cpp:1195`). The fix is one
line: call `base::Animate(level)` at the top of `Church::Animate` (and the matching `Deanimate`).
A script cannot work round it: `setPos` refuses map objects (`Game/Commands/GameStateExtGrp.cpp:1760`),
so the church cannot even be sunk. Until it is fixed the three church models (`Church001` to
`Church003`) are the one kind of building he walks through with nothing happening to it.

## 1. Footfall events (largest gain)

Today: no script can read a bone, a memory point or the phase of a clip, so `loop.sqs` works a
footfall out from the ground covered since the walk clip started (one foot per 0.22 x height, right
foot first, at a fixed offset from his origin). That is in step with the legs only if the walk state
has `interpolationRestart = 1`: without it the walk takes over whatever phase the idle clip was at
(`World/Entities/Infantry/SoldierOldMove.cpp:337-349`). A swap back to a clip that is still blending
out (`World/Entities/Infantry/SoldierOld.cpp:1335-1350`) keeps its phase too, and the script cannot
tell the two cases apart. The dust, the deaths and the flattened vehicles all hang off this guess.

Wanted: the engine already knows the moment. `SoldierOldMove.cpp:186-201` crosses `soundEdge1` /
`soundEdge2` of the move and knows which foot (`left`). Raise an event there for a flagged type:
`kaiju addEventHandler ["Footfall", {...}]` with `[unit, "L" or "R", world position of the foot]`,
the position taken from the memory points `stopaL` / `stopaP` (the proxy has them). That needs
`soundEnabled = 1` and the two edges set in the walk state. With it the crush, the roar feature's
thump and the camera shake all hang off the real step.

A smaller form that would already help: `unit selectionPosition "name"` (a memory point, animated)
and `unit modelToWorld [x, y, z]`. Neither exists in this command set. They would also give the real
hand position for the swat and the real toe position for item 2.

## 2. His legs collide with vehicles, and a hull is a wall

Measured (probe 2, `runs\probe2`): a live T-80 standing in his left foot lane stopped him dead. He
walked 14 m, the foot met the hull, and he was put back 6 m, again and again for a minute. An AI Man
skips statics but not vehicles (`World/Simulation/Collisions.cpp:187-193`,
`World/Entities/Infantry/SoldierOldSim.cpp:938-1040`: he is moved out of the hull by the full
penetration every frame), and a wreck keeps its geometry. Mass plays no part on his side.

Men do it too. Measured (`runs\gate3x3-1111-212107\r1_crush_walk`): a civilian standing where his left
foot came down shoved the 20 000 t giant 3.3 m sideways and 0.16 m up. A fallen body does not.

The script's workaround: a man in a foot lane is killed a moment before the foot is on him, so he is
down when it lands, and a vehicle in a foot lane is taken out as the toes of that foot reach it
(worked out from the stride again): flattened, which means deleted and replaced by a stock static
wreck model pressed into the ground, or wrecked and knocked sideways out of the lane. Both happen a
moment before the foot would really touch, and both depend on the footfall guess of item 1.

Wanted: a type flag (or `unit setCollisionWith "vehicles", false`) that makes the giant ignore
vehicle geometry in `Man`'s collision pass, plus a native crush of what his foot boxes overlap
(item 3). Then a tank really is under the foot when it dies.

## 3. A native crush pass (the per-frame cost, and "never a house inside his leg")

Today: every 0.5 s of walking the script asks `nearestObjects` for everything within about 25 m of a
point ahead of him and walks the result in script (roads and bushes included, they are most of what
comes back in a town). It cannot measure a building, so it assumes a half-size of 0.18 x height
(7 m) and marks anything whose centre is inside the leg strip widened by that. A long building (a
hangar) is caught late; a shed beside him falls although the leg missed it. Frame cost was not
measured (the machine was shared; any number would be contended).

Wanted: a native pass per step over the object grid (the pattern at
`Game/Commands/GameStateExtUi.cpp:614-630`) that tests each object's own bounding box against the two
foot boxes and the leg capsules, with no visibility ray, and calls `Object::SetDammage` or
`LocalDammage` with him as owner. Also: `Object::Destroy` (`World/Entities/Weapons/Dammage.cpp:544`)
could take a start delay and a fall direction, so the dust can lead the collapse natively and a tree
falls away from the leg (today it rotates about its own X axis, `Dammage.cpp:466-481`, so it falls
whichever way the tree happens to face).

A smaller form: a script command `boundingBox obj` or `sizeOf obj`. Not in this command set.

## 4. Knockback without a projectile

Today: `setVelocity` is the only push a script has. Measured (probes 1 and 2):

- a UAZ given 11 m/s flies and slides about 16 m: good;
- a live T-80 given 13 m/s slides 6 m, given 25 m/s 18 m; `setPos` cannot lift a tracked hull off the
  ground first, so it never leaves the ground; a destroyed T-80 given 45 m/s slides 80 m;
- a man ignores it completely, on the ground or lifted 1.2 m into the air (he hung there and sank
  half a metre in three seconds). So men near a footfall are "thrown down" by an order to go prone,
  not thrown;
- there is no torque: a kicked tank slides, it does not tumble, and a struck helicopter leaves in a
  straight line.

Wanted: `obj addImpulse [force, torque]` around `Entity::AddImpulseNetAware`
(`World/Simulation/Simul.cpp:528, 544`), or let `Landscape::ExplosionDammage` push when `shot` is
null (`World/Simulation/Collisions.cpp:1087, 1153`: both impulse branches are `if (veh && shot)`).
For men, an impulse above five times their mass already ragdolls them
(`World/Entities/Infantry/SoldierOldSim.cpp:1074-1097`); script just cannot apply one.

## 5. Damage clamp and hit reporting for the type

Today (numbers in `kaiju-notes\features\crush\NOTES.md`, "Being shot"):

- `allowDammage false` zeroes every hit and with it every event (`AI/VehicleAICombat.cpp:75-80,
  118-123`), so he must stay damageable and `hurt.sqs` heals him every 0.2 s with `setDamage 0`.
- The `Hit` event needs 0.05 total damage from one hit (`AI/VehicleAICombat.cpp:216-226`). On a body
  this size with `armor = 1000` only a Maverick or a bomb reaches that, so the script reads the rise
  in `getDammage` instead and takes the direction from the last heavy `Fired` it heard. That is a
  guess when two shooters fire within the same four seconds.
- One laser-guided bomb is worth 0.89 of his life, and a Maverick on the head kills through the zone
  rule unless the zone armour is raised (the crush addon variant raises it). The script undoes a
  death with `setDamage 0` (measured: he stands up and walks again;
  `World/Entities/Infantry/SoldierOldMove.cpp:1309-1330`), but the death has already run its side
  effects (the `Killed` event, the group's bookkeeping, the first frames of the die clip).

Wanted, in `EntityAI::HitBy` / `Object::LocalDammage` for a flagged type: (a) total damage and every
hit zone never pass a ceiling (config `kjDamageCap`, or `unit setDamageCap x`); (b) an event for
every hit, without the 0.05 floor, carrying `[unit, who, ammo class, raw hit value, world position of
the impact]`. The impact position gives the real direction and height for the flinch.

## 6. Squashing a wreck

Today: the engine has no per-object scale reachable from script, a destroyed tracked hull cannot be
pressed into the ground (measured: `setPos` with a negative height leaves a T-80 on the surface, a
UAZ stays 1.2 m in), and item 2 means the hull must go anyway. So a flattened vehicle is a stock
wreck model (`JeepWreck1`, `UralWreck`, `M113Wreck`) pressed into the ground: a T-80 becomes the hulk
of an M113. `Object::Scale()` exists and feeds the draw (`World/Scene/Object.hpp:553`). Wanted:
`obj setObjectScale [x, y, z]`, or at least a vertical squash for dead vehicles, so a flattened tank
is its own hull at a third of its height.

## 7. Grid margin

`ObjRadiusRectangle` widens every grid query by a constant 25 m (`World/Simulation/Collisions.cpp:51-55`).
The proxy stays inside that in plan view, so nothing was seen to fail here. It matters as soon as the
final model has a tail or reaches forward: shells aimed at the far end of him would pass through, and
`nearestObjects` centred on a hand would not return him. Wanted: the margin taken from the largest
live dynamic radius, or the giant filed in every cell he overlaps.

Related, measured: `createVehicle` will not place anything within about 16 m of him (it looks for
free room around his whole radius; the tests make vehicles far away and `setPos` them). A spawner
for his victims has to do the same.

## 8. Credit for the kill

Today: `setDamage` passes no owner (`World/Entities/Weapons/Dammage.cpp:422-451`; for a map object it
also logs "SetDammage applied on non-EntityAI" every time). So nobody is told that he did it: no
`Killed` event with him as killer, no reveal to the victim's group, no panic from a witnessed death.
Wanted: `obj setDamage [value, who]`, routed to `LocalDammage` / `Destroy` with the owner set, and
without the error line for map objects.

## 9. Walking straight

Not crush's to build, but crush shows it worst: the stock AI plans a path round every house and tree
(survey, section 5), so with `doMove` he threads the streets and only shaves what stands beside his
route. Measured: he walks straight only when driven with `playMove "DedWalk"`, and then at half the
clip's speed (1.5 m/s, the state's `relSpeedMin`). An obstacle-blind planner flag or a native
straight steer is what makes him walk through a block.

## 10. Camera shake

Every collapse and every footfall wants it, and the engine has none (survey, section 6). A tremor
source with a position, a strength and a falloff, fed by item 1.
