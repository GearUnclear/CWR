# Seated-man deletion: move him out of the seat first

Verified 2026-10-14 on branch `occupier-forces` (working tree after `c5cfa44`),
on `build/agent-rwdi` against `tmp/remaster-shadow-of1`. Paths are relative to
`arma_CWR/engine/Poseidon/`.

## Symptom

Engine code deletes a crewman, a rider or a corpse with `::DeleteVehicle`
while he sits in a vehicle. Nothing looks wrong: the hull goes away and the
man is gone from the screen. But the man object lives on, invisible, and is
written into every save. Repeated spawn and despawn (ambient traffic,
counter-attack crews) piles these up for the whole session.

Observed with a throwaway Trident probe on Abel: five traffic hulls
force-spawned (civ, patrol, convoy truck plus escort, a second civ and patrol
with a killed driver and gunner), the player moved 4 km away, the hulls
despawned. Before the fix every crew reference the script held was still
non-null after its hull was gone: live crew 9 of 9, dead crew 3 of 3. After
the fix: 0 of 9 and 0 of 3.

## Why it happens

Observed in the code:

- `::DeleteVehicle` (`Game/Commands/GameStateExtWorld.cpp:334`) on a person
  detaches his brain and then only calls `SetDelete()` (line 400).
- A man in a seat is not in the world's `_vehicles` list but in
  `World::_outVehicles` (`World/World.hpp:236`). Nothing removes flagged
  entries from `_outVehicles`; it is serialised as-is
  (`World/WorldImpl.cpp:1685`).
- Deleting the hull instead does not help a corpse: the hull's eject path
  (`Eject`, `Game/Commands/GameStateExtWorld.cpp:312`) only calls `SetDelete()`
  on a man without a brain, again leaving him in `_outVehicles`.
- `Transport::GetOutAny` (`World/Entities/Vehicles/TransportCrew.cpp:947`)
  moves the man from `_outVehicles` back into `_vehicles`, where a flagged
  entity is dropped normally.

Inferred: the simulation pass over `_vehicles` is what actually frees a
deleted entity, so anything flagged while sitting in `_outVehicles` is never
freed.

## What to do

Before deleting a man, take him out of his seat with the matching
`GetOutDriver` / `GetOutGunner` / `GetOutCommander` / `GetOutCargo` at the
hull's transform and `sound = false`, then delete him. Before deleting a hull,
do the same for every occupant (dead ones included: a corpse whose brain is
gone is found by comparing the hull's seats, not through `Brain()`).

Reference implementations:

- `Game/Guerrilla/CounterAttack.cpp`, `EngineOps::LeaveVehicle` and
  `DeletePerson`.
- `Game/Guerrilla/Traffic.cpp`, `DeleteTrafficPerson` and `DeleteTrafficHull`
  (which also find a seated corpse by its hull when it has no brain).

To check a deletion path, hold the crew in script variables, let the hull go
away, run a few frames and test `isNull` on each man: a leaked man stays
non-null, a properly deleted one becomes `objNull`.
