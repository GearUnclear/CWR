# BUS-STOPS.md - bus passenger pick-up/drop-off (design stub)

**Status: STUB, unscheduled.** Nothing here is implemented. What HAS landed
(2026-09-01, same change as this file): the Resistance-era `Bus` hull is in
the `civVehicles[]` traffic rosters of `Guerrilla.Abel`, `Market.Abel`,
`Guerrilla.Sinai` and `Guerrilla.Lebanon80`, so buses already drive
town-to-town as ordinary (empty) civilian traffic. This stub is the plan for
making them *carry people*. Roadmap item: `traffic-bus-stops`.

## Why

From the player's seat a bus is the strongest civilian-life signal a road can
carry: it is big, slow, legible at range, and implies passengers, schedules
and stops. It is also the highest-stakes collateral object ambient traffic
can offer (hitting a bus at a checkpoint ambush should hurt), which plugs
straight into the civilian kill ledger.

## Verified engine facts this leans on

- `class Bus` ships with Classic 1.99: declared in `O.pbo`'s
  `CfgPatches BIS_Resistance` units[] (the earlier "absent from 1.99" note
  probed only `Bin\CONFIG.BIN`, which cannot see addon-pbo configs).
- Dedicated crew animation states exist: `busDriver*` and `busCargo*`
  (`BusPassanger*.rtm`) in `O.pbo`, wired via `CfgVehicleActions`, so seated
  passengers render correctly with zero animation work.
- `Traffic::CreateCrewman(grp, type, pos, veh, GIPCargo)` already seats
  cargo crew (used for military convoy riflemen); civilian passengers are
  the same call with a `civClass<N>` body.
- The park/dwell/depart machinery (`TSParking`/`TSDwelling`/`TSDeparting`)
  is the proven stop-and-resume choreography: brake via `IssueGroupStop`,
  timed dwell, animated GetIn re-board, `DepartTimeout` + perception-gated
  `NativeMoveIn` fallback.
- `O.pbo` carries a `Misc\busstop.paa` texture, so a bus-stop sign/shelter
  world object likely exists (Czech-named model, probably `zastavka`-ish;
  confirm with `PoseidonTools` before relying on it).

## Sketch

1. **Route shape.** Buses only make sense CITY-to-CITY. Add a per-class (or
   per-kind) route bias to the spawn pick in `SpawnEntry`: `Bus` is chosen
   only when the rolled route is CITY-CITY and above a minimum leg length;
   conversely bias `Tractor` to short/village legs. This is the ~20-line
   route-weighted-pick change; zone `isCity` and both endpoints are already
   in scope at the pick site.
2. **Passengers at spawn.** A bus spawns with 1-4 `civClass<N>` cargo
   passengers (count rolled per spawn, fewer at night). Every passenger gets
   the `driverKilled` killed-EH expression so the civilian kill ledger sees
   bus casualties. Prerequisite that benefits ALL civ traffic: generalize
   the commandeer bail from driver-only to the all-crew dismount-and-flee
   loop that `AbandonEntry`/`DespawnEntry(keepHull)` already use, so a
   stopped bus empties believably instead of leaving frozen passengers.
3. **The stop.** A new `TSBusHalt` state entered when a driving bus passes
   within a threshold of an en-route CITY zone centre's nearest road point:
   brake (`IssueGroupStop`), dwell 15-30 s, passenger delta, resume the leg
   (the `TSPanicked` resume shape: re-`IssueRoute`, stall clock reset).
4. **Passenger delta under the perception gate.** Boarding/alighting is the
   hard part and must respect issue #53 semantics (no visible pop-in):
   - Alight: dismount 0-N seated passengers (the park dismount idiom), walk
     them away from the road (`IssueSoloMove`), hand them to the fleeing
     table at walking pace with a long leash; they despawn under the
     existing perception-gated cleanup. Works fully observed.
   - Board: only plausible when the walker already exists. Cheapest honest
     version: spawn the boarders as roadside walkers near the stop BEFORE
     the bus arrives (they are the alibi), walk them to the door, seat via
     the animated GetIn; if boarding is blocked while observed past a
     timeout, they simply stay walkers (no teleport-seat in view, ever).
   - Degenerate v1 (acceptable): alight-only. Passengers get off along the
     route; the bus never picks anyone up in view. Half the fiction, none
     of the boarding choreography risk.
5. **Interactions.**
   - Commandeer: works as any civ car once the all-crew bail lands; a
     hijacked bus is a great undercover troop carrier, which is a feature,
     not a bug (civilian-typed hull, anonymous per the vehicle rule).
   - Danger response: bus reacts like any TKCiv entry; a bail empties the
     whole bus (all-crew loop again), which is the massacre-optics moment
     the kill ledger should price.
   - Caps: a bus counts 1 against `maxCiv`; no special cap in v1.

## Open questions

- Bus-stop anchor: zone-centre nearest road point vs a real bus-stop world
  object per town (nicer, needs a per-island object scan or authored data).
- Does the `Bus` hull's turning circle cope with every island's village
  roads, or does it need a route blacklist (probe in play; the blocked
  ladder's retry/U-turn is the safety net)?
- Passenger identities: reuse `civClass1..3` bodies or draw from the town
  civilian pool so a passenger "belongs" somewhere (ledger attribution).
- Whether TSBusHalt should suppress the park roll at arrival (a bus that
  terminates should park at the town stop, not despawn mid-street).

## First slice (when scheduled)

Route-biased pick (Bus on long CITY-CITY legs only) + spawn-with-passengers
+ per-passenger killed-EH + all-crew commandeer bail + alight-only stops.
Boarding choreography and bus-stop objects come after an in-game probe.
