# Occupier air: the phase 6 spike

The opening spike of phase 6 in [OCCUPIER-FORCES.md](OCCUPIER-FORCES.md)
(section 14, the "Air" item; D3, D4, D6, D17, D20, D21; sections 3.1, 3.2,
6.5, 7.3, 8.2, 12.2). It answers one question before any air is wired into
the director:

> Can an AI transport helicopter, born unseen and gated at altitude, fly to a
> town, land within 100 m of its LZ in at least 80 % of trials, unload its
> troops reliably and depart; and can a jet be sent on a sortie (search the
> target, limited reveal), then leave and be deleted crew-first?

**VERDICT: GO.** On the final build, 18 of 18 helicopter insertions landed
within 100 m of their LZ (worst 20 m, median 5.5 m), unloaded 8 of 8 troops
alive and departed, and every airframe was removed crew first; 5 of 5 tri
runs of `guerrilla_forces_heli_insertion` passed. Over every build of the
spike, 44 of 45 insertions touched down within 100 m (97.8 %, worst 23 m);
the one miss was a wave-off fixed in build b3. 6 of 6 jet sorties passed
(`guerrilla_forces_jet_sortie`). The departure needed engine-level fixes
(failure modes 3 and 5 below), all trialled clean on the final build.

The module is native and self-contained (`engine/Poseidon/Game/Guerrilla/
OccupierAir.{hpp,cpp}`, commands in `OccupierAirCommands.cpp`). It is NOT
wired into `CounterAttack` or the director: phase 6 proper calls its API.
The only world hooks are `OccupierAir::Instance().Simulate(deltaT)` in
`World.cpp` (idle with no records) and `Clear()` in `WorldInit.cpp`. Records
are transient (not saved).

## What works

- **Airborne birth** at any point, over the sea beyond the map edge
  included, at a chosen height above ground. The Mi17 is flying at once
  (about 60 m/s cruise at 100 m) and the Su25 at 0.66 x its max speed
  (171 m/s).
- **The altitude-aware gate** (`AirViewer`, `gmAirExposure` / `gmAirUnseen`):
  the Traffic #53 camera, band and terrain ray, evaluated at the aircraft's
  height. The sea is not special (off-map renders); the audible floor for
  aircraft is 800 m (`airAudible`). Every birth point tried rated HIDDEN at
  its birth altitude.
- **The LZ choice** natively on the 300..500 m ring around the target, the
  side away from a threat first (`gmAirInsert ... "away"`). Each candidate
  snaps to the centre of its 50 m terrain cell, and that cell must pass the
  engine's own landing-site test (no water, not full, no objects, gradient
  below 4) plus no forest flag and a sampled slope at most 0.25 across 22 m.
  Every target tried found an LZ within 16 candidates.
- **The landing**: an invisible `HeliHEmpty` pad in GWorld's BUILDINGS list
  at the LZ, then `Transport::LandStarted(LMLand)` once the helicopter is
  within 450 m of the pad, re-issued every 5 s until down.
  `HelicopterAuto::FindStopPosition` picks the pad. Every touchdown was
  within 23 m of the LZ, most within 7 m.
- **The unload**: crew and cargo are two groups; the cargo gets out at once
  with `OrderGetIn(false)`, `AllowGetIn(false)`, `DoGetOut`, then a per-unit
  `AIUnit::UnassignVehicle`, and walks on the target with a Move. 8 of 8
  bodies out and alive in all 44 touchdowns.
- **Departure and removal**: the landing order is ended
  (`Transport::LandFinished`, the pilot put to Wait when still Stopping or
  Stopped), then a fresh `Command::Move` home lifts it off (the rotor
  restarts on a 2+ point path). It is removed where `UnseenAt` holds
  at its altitude for 2 s: the crew FIRST (any cargo still seated with it),
  the hull one step later, the pad with it. No crew body was ever left in
  the world.
- **The jet sortie**: Move to the target, an orbit of Moves 900 m out
  (8 points, a leg every 20 s) in COMBAT / open fire for `sortieTime`, then
  home and removed the same way. It never touched the ground (no ILS, no
  taxi). The limited reveal (`AIGroup::AddTarget` with accuracy below 1,
  sightings younger than 60 s only) is refused for an old sighting or an
  exact fix and passed otherwise.

## The recipe (exact calls)

Helicopter insertion (`OccupierAir::SpawnInsertion`):

1. `NewNonAIVehicle(type)`, check `IsKindOf(VTypeAir)`; orient with
   `SetUpAndDirection(VUp, heading)` and `SetTransform` FIRST.
2. `SetFlyingHeight(altAgl)` (writes `_defPilotHeight`), then
   `pos.Y += MakeAirborne()` (engine on, rotor up, returns the height).
3. `SetTransform`, `Init`, `GWorld->AddVehicle` (plus the network create).
4. Crew group: `CreateUnit` + `NativeMoveIn(GIPDriver)` (and the gunner
   seat when the type has one), the config `crew` class or the side pilot.
5. Cargo group (a SECOND group): `CreateUnit` + `NativeMoveIn(GIPCargo)` +
   `SetState(InCargo)`, faction classes from `ZoneRegistry::FactionSquad`.
6. Crew CARELESS / GREEN, cargo SAFE.
7. Pad: `NewNonAIVehicle("HeliHEmpty")`, `PlaceOnSurface`, `SetTransform`,
   `Init`, `GWorld->AddBuilding` (NOT AddVehicle: FindStopPosition walks the
   buildings list only).
8. `Command::Move` to the LZ through `AIGroup::IssueCommand` to the driver.
9. Within `landOrderDist` (450 m, below FindStopPosition's 500 m pad
   radius): `LandStarted(LMLand)`; re-issued every `LandRetry` (5 s) until
   down (see failure 2).
10. Touchdown = height above ground <= 5 m and speed <= 3 m/s for 1 s; then
    the unload (above, with `AIUnit::UnassignVehicle` per cargo unit), and
    when no cargo is seated: `LandFinished()`, the pilot `SetState(Wait)` if
    Stopping or Stopped, then the Move home.
11. Removal where unseen: `::DeleteVehicle` on every crew body (and any cargo
    still aboard), the empty groups `RemoveFromCenter`, then the hull, then
    the pad.

Jet sortie (`OccupierAir::SpawnSortie`): steps 1 to 4, AWARE / RED, Move to
the target; within 1.25 x the orbit radius COMBAT / RED and the orbit Moves;
after `sortieTime` a Move home; removal as step 11.

## API for phase 6 proper

- `AirInsertionPlan { heliClass, cargo, birthPos, birthAlt, lz, target, side }`
  and `OccupierAir::PickLz(target, awayFrom*, minR, maxR, lz, why)`, then
  `SpawnInsertion(plan, id, why)`.
- `SpawnSortie(planeClass, birthPos, birthAlt, target, sortieTime, side, id, why)`
  and `Reveal(sortieId, target, ageSeconds, accuracy)`.
- `ExposureAt(viewer, posWithAlt)` / `UnseenAt` with `EngineViewer()` for the
  birth search (the off-map edge and the airfield).
- Reading back: `GetInsertion(i)` / `GetSortie(i)` (state, LZ offset, cargo
  group, removal order), `LiveAirframes()`, `LiveGroups()` for the 40-group
  budget.
- Script levers (tests and drivers only): `gmAirInsert`, `gmAirSortie`,
  `gmOccupierAir`, `gmAirExposure`, `gmAirUnseen`, `gmAirAbort`,
  `gmAirReveal`, `gmAirTune` (column layout in `OccupierAirCommands.cpp`).

## Trials

Remaster shadow, `Guerrilla.Abel` (occupier EAST), director off
(`gmForcesTune ["enabled", 0]`), player at the Camp, captive and
damage-shielded, default view distance, daylight. Each run is a fresh game
launch with `--retries 0`.

Builds (each adds to the one before): **b1** first compile, landing
ordered at 450 m; **b2** LZ probe on the cell centre (failure 1); **b3**
landing order re-issued (failure 2); **b4** per-unit unassign (failure 3);
**b5** `LandFinished` before the Move home (failure 5, first half); **b6**
the pilot released to Wait (failure 5): the final build.

`guerrilla_forces_heli_insertion` (A: south edge [8200, 150] to Le Port,
LZ [7825, 3325], [8539, 3329] in b1; B: west edge [150, 6100] to Houdan,
LZ [7175, 6475] picked away from the Camp, landing in the player's view).
Offset = 2D metres from the LZ at touchdown. "Removed" = crew deleted, then
the hull one step later (removal order 1, no crew body left).

| Run | Build | A offset | B offset | Unload (A / B) | Departure and removal | tri |
|---|---|---|---|---|---|---|
| 1 | b1 | 1 m | no LZ (failure 1) | 8/8 / - | A removed 3 s after lift-off | red (B refused) |
| 2 | b2 | 1 m | none: wave-off, hovered 100 m up, timeout | 8/8 / - | A removed; B removed as failed | red (game stopped by me during an assert retry, not a crash) |
| 3 | b3 | 2 m | 3 m | 8/8 / 8/8 | both removed crew first | all 93 steps OK, then hung: no `triEndTest` (added) |
| 4 | b3 | 1 m | 7 m | 8/8 / 8/8 | B re-landed 285 m away and stayed (failure 3) | stopped by me |
| 5 | b4 | 0 m | 7 m | 8/8 / 8/8 | both removed crew first | PASS |
| 6 | b4 | 1 m | 6 m | 8/8 / 8/8 | both removed crew first | PASS |
| 7 | b4 | 1 m | 23 m | 8/8 / 8/8 | both removed crew first | PASS |
| 8 | b4 | 7 m | 5 m | 8/8 / 8/8 | both removed crew first | red: test lift-off wait (failure 4) |
| 9 | b4 | 7 m | 2 m | 8/8 / 8/8 | both removed crew first | red: test lift-off wait (failure 4) |
| 13 | b5 | 7 m | 3 m | 8/8 / 8/8 | B held a 30 m hover over the LZ (failure 5) | stopped by me |
| 14 | b6 | 1 m | 9 m | 8/8 / 8/8 | both removed crew first | PASS |
| 15 | b6 | 6 m | 2 m | 8/8 / 8/8 | both removed crew first | PASS |
| 16 | b6 | 2 m | 9 m | 8/8 / 8/8 | both removed crew first | PASS |
| 17 | b6 | 9 m | 20 m | 8/8 / 8/8 | both removed crew first | PASS |
| 18 | b6 | 7 m | 1 m | 8/8 / 8/8 | both removed crew first | PASS |

(Runs 10 to 12 were started on b4 and stopped for the b5 rebuild before
any landing; they carry no data.)

Variety probe (a scratch test, not committed): four Mi17s at once from the
east edge to the first four zones 1.5 to 6 km from the Camp: the Compound
[9715, 3940] (LZ 361 m out), the Harbour [8410, 3690] (415 m), the Airfield
[8030, 9905] (409 m) and the town at [7144, 8963] (318 m).

| Run | Build | Compound | Harbour | Airfield | Town | Unload | Departure and removal |
|---|---|---|---|---|---|---|---|
| V1 | b4 | 1 m | 1 m | 5 m | 3 m | 8/8 each | three removed; the Airfield one hovered at its home point inside the view cone (failure 6) |
| V2 | b4 | 3 m | 7 m | 1 m | 5 m | 8/8 each | the town one re-landed 110 m away (failure 5); stopped by me |
| V3 | b6 | 3 m | 7 m | 2 m | 3 m | 8/8 each | all removed crew first |
| V4 | b6 | 2 m | 6 m | 5 m | 7 m | 8/8 each | all removed crew first |

`guerrilla_forces_jet_sortie` (Su25, east edge [12700, 3500] at 250 m to
Le Port, 90 s sortie, player facing north-west):

| Run | Build | Search orbit reached | Reveal (old / exact / fresh) | Touched the ground | Removed (crew first) | tri |
|---|---|---|---|---|---|---|
| 1 | b4 | 21 s | refused / refused / passed | no | 115 s | PASS |
| 2 | b4 | 21 s | refused / refused / passed | no | 114 s | PASS |
| 3 | b4 | 21 s | refused / refused / passed | no | 114 s | PASS |
| 4 | b4 | 21 s | refused / refused / passed | no | 114 s | PASS |
| 5 | b4 | 21 s | refused / refused / passed | no | 114 s | PASS |
| 6 | b6 | 21 s | refused / refused / passed | no | 114 s | PASS |

The jet orbits at 270 to 310 m above ground and 115 to 170 m/s.

## Failure modes found (and what was done)

1. **No LZ near Houdan** (trial 1): the first probe refused any candidate
   whose 22 m disc touched a 50 m cell holding any object, which rejects
   most of a wooded island. Fixed: the candidate snaps to its cell centre and
   only that cell must be object-free (the engine's own landing test); a
   clean 50 m cell leaves at least 25 m to the nearest object.
2. **Wave-off over the pad** (trial 2, insertion B): `LandStarted` was issued
   while the approach `Command::Move` was still running. The Move ended near
   the pad, took the pilot out of Stopping, and he climbed back to 100 m and
   hovered until the 150 s landing timeout. Fixed: the landing order is
   re-issued every 5 s until down (a no-op while the pilot is still
   Stopping). Since then no wave-off outlasted a re-issue.
3. **Uncommanded second landing** (trial 4, insertion B): after a clean
   unload and lift-off the helicopter flew 285 m and landed beside the
   dismounted squad, and stayed down: the cargo units were still ASSIGNED to
   the hull as cargo, which the AI reads as a pickup request. Fixed: each
   cargo unit's own assignment is dropped (`AIUnit::UnassignVehicle`; never
   `AIGroup::UnassignVehicle`, which clears the crew's seats too). Not seen
   since.
4. **Removal before the lift-off mark** (trials 8, 9, test only): the
   gate may rightly remove a departing helicopter 2 s after lift-off at
   20-30 m when nobody looks, before the record's 20 m lift-off stamp is
   taken. The tri test now accepts "lifted off or already removed".
5. **Hover or re-landing after the unload** (runs V2 and 13): the touchdown
   is declared at 3 m while the pilot may still be Stopping under the
   landing order. With the order left standing, the Move home lifted him off
   and the standing LMLand put him down again at the nearest free spot,
   where he stayed. Ending only the order (`Transport::LandFinished`, build
   b5) left the pilot in Stopping with no reason to finish it, so he held a
   30 m hover over the LZ. Fixed in b6: before the Move home,
   `LandFinished()` and, when the pilot is Stopping or Stopped,
   `AIUnit::SetState(Wait)` (the engine's own Stopped to Wait pass); a Move
   home is also re-issued every 45 s while it stays down after a lift-off.
   Clean in all 18 b6 insertions.
6. **Removal in the view cone waits** (run V1, the Airfield insertion): an
   airframe hovering at its home point inside the player's view cone is
   never `UnseenAt` (the Traffic `CanExposeDespawn` rule, as 3.2 prescribes)
   even 6.5 km away, far past the draw range; only `departTimeout` (900 s,
   and past the band) removes it. See "Open for phase 6".
7. **Touchdown at 3 m.** Touchdown is declared at <= 5 m and <= 3 m/s held
   for 1 s; the Mi17's origin sits about 3 m above the ground when it is
   down, so the logged "3 m up" is on the ground. No cargo body was hurt.

## Open for phase 6

- **The in-cone removal rule for aircraft.** Ground despawn never removes in
  the forward view; for an aircraft past the band (object cull plus margin)
  nothing is drawn, so `UnseenAt` could accept "past the band in 3D" even
  in the cone, the same leg `ExposureAt` already uses for birth. This
  changes the 3.2 wording, so it is an owner decision; until then
  `departTimeout` should drop well below 900 s.
- **The jet's actual strike.** The trials prove birth, search pattern,
  limited reveal, departure and removal; they do not assert damage on a
  target. Town strikes (7.3, D21) and the bought-when rule are untouched.
- **Births**: the spike takes the birth point as given; the off-map edge
  choice (`forcesOffMapEdge`) and the airfield source are phase 6 work, on
  top of `ExposureAt`.
- **Descriptor keys** (`forcesTransportHeli`, `forcesAttackHeli`,
  `forcesJet`) and the library entries: none yet. Classes used: EAST Mi17
  and Su25 (`su25.pbo`, loaded without an `addOns[]` entry on the Remaster).
- **Attack helicopters** (Mi24, Cobra) as sorties: the code path is the
  jet's (no landing), not trialled.

## Phase 6 proper (6A: air in CounterAttack)

The spike's module is now wired into `CounterAttack`
(`engine/Poseidon/Game/Guerrilla/CounterAttackAir.cpp`). The contract with
the director is `CounterAttackOrders.hpp`: the kinds `CAKStrike` and
`CAKAlarm`, the order fields `airOrigin`, `strikeZone` / `strikePos` and
`alarmZone` / `alarmPos`, and the events `CAOpAirborne`, `CAOpLanded`,
`CAOpStruck` (once per strike, with its zone and position) and
`CAOpAlarmAnswered`.

- **Packages.** `ForcePackages::DefaultPackages` now includes the air
  packages; one is offered only when the faction descriptor authors its class
  (`forcesTransportHeli`, `forcesAttackHeli`, `forcesJet`: EAST Mi17 / Mi24 /
  Su25, WEST UH60 / Cobra / A10, and the @LoBo rosters where a class of the
  right side exists, in `config/guerrilla-factions.hpp` and
  `config/lobo-factions.hpp`). Prices 10 / 18 / 22, x1.5 off map
  (`PackageRequest::airOffMap`); speed 0.8 x `maxSpeed` on a straight line;
  the transport carries `min(12, transportSoldier)`. The strike airframes
  carry nobody (`SeatCap` 0), so a ground operation never chooses one.
- **Insertion.** An ordinary record (RETAKE, BASE, HQ_ASSAULT, REINFORCE,
  ALARM) with `package = FPTransportHeli`. Abstract, it flies the straight
  line from `airFrom` (the airfield, else the off-map edge, fixed at launch).
  When it must go live it is born AIRBORNE (100 m up) at the first point the
  altitude-aware gate rates hidden, searched back from its progress point to
  the origin, then 1500 to 5500 m beyond the map edge (`forcesOffMapEdge`,
  default the edge nearest the target). The LZ is `OccupierAir::PickLz`'s,
  the side away from the reported fighters' centre (OccupierForces
  sightings, D9). The cargo group becomes the record's live squad once
  unloaded and walks the straight line from the LZ; the airframe goes to the
  record's `AirHome` list and is refunded (`CAOpHullRefund`) once OccupierAir
  removed it crew first, or booked lost (`CAOpHullLost`). No LZ, or no
  airframe: the force goes on foot, its price refunded.
- **Failure and re-plan.** A touchdown more than 100 m from the LZ (or a
  timeout, or no pilot) fails: OccupierAir flies the airframe home with its
  cargo and removes it unseen; the record goes back to abstract flight once
  (the airframe re-created at the next birth, no refund), then is cancelled
  unseen. A shot-down airframe is lost; whoever got out fights on.
- **Birth chain (3.1 step 4).** After the next base back, a funded ground
  operation with no unseen birth point is re-planned as a helicopter
  insertion when the catalogue offers one, its seats hold the bodies and the
  pool pays it (`OccupierForces::Spend`): the ground hull goes back through
  the ledger, the airframe's price is spent and later refunded directly
  (`Record::airReplanned`), not through the director's row.
- **Strikes (D17).** A `CAKStrike` record carries no bodies. Abstract, it
  reaches `strikePos` on its clock; live (the target in the bubble, the
  budgets free) it is an OccupierAir sortie born airborne (250 m for a jet)
  under the same gate, orbiting for `forcesSortieTime` (300 s, now read from
  config with `forcesOffMapEdge`), with the limited reveal every 10 s of a
  sighting younger than 60 s (resistance AI within 100 m, accuracy 0.5,
  never the player). `CAOpStruck` goes out once, live or abstract; the
  director applies the 7.3 effects on it.
- **Alarm answers (8.2).** A `CAKAlarm` record travels to `alarmPos` like a
  top-up column, emits `CAOpAlarmAnswered` on arrival, and live it holds
  there until the zone is below RED with no resistance presence for 60 s
  (`AlarmQuietHold`) or the live timeout, then joins the garrison; abstract,
  it tops the garrison up at once and the surplus goes to the pool.
- **The open item, decided.** For aircraft, `OccupierAir::UnseenAt` also
  accepts "beyond the perception band in 3D from the camera", inside the view
  cone too (nothing is drawn there); `departTimeout` (900 s) stays as the
  backstop. Removal order is crew, then hull, then pad (the pad no longer
  goes at departure), and every crewman leaves his seat before he is deleted
  (the `EngineOps::LeaveVehicle` idiom), so none lingers in the world's
  out-vehicle list or reaches a save.
- **Persistence.** The record's air fields, the `AirHomes` list and
  OccupierAir's insertions and sorties are saved inside the `CounterAttack`
  block (refs resolve on the second pass; a pad missing after a load is
  re-created). `guerrilla_forces_air_save_reload.seq` saves an insertion
  mid-flight and resumes it in a fresh process.
- **Not done.** An attack helicopter as the escort of an insertion (an
  insertion carries one airframe; the strike path covers the attack
  helicopter on its own). The descriptor lint passes the `forces*` keys
  through without checking the class exists (ZoneRegistry's key predicates do
  not list them); a typo shows only as BuildCatalogue's runtime WARN.
