# TEMP handoff: ambient road traffic (native Traffic service)

> **Delete this file if you are reading it after 2026-09-14.** It is a
> short-lived working note, not documentation. Everything durable already
> lives in `guerrilla-mode/STATUS.md`, `guerrilla-mode/ARCHITECTURE.md`,
> `guerrilla-mode/README.md`, `roadmap/roadmap.yaml` and the commit messages.
> This file is untracked scratch: do not commit it, do not cite it.

Written 2026-08-22 (session that landed the feature).

## Status: DONE and landed

Two commits on `main`:

- `f255bef` guerrilla: native ambient road traffic + road script commands
- `6a8bdeb` roadmap: log the ambient-traffic + nearestRoads landing

Roadmap items `ambient-traffic` (new) and `nearest-roads-command` (road half)
are both `done` with `commits: ["f255bef"]`. Fork issues touched: #33 (road
commands), #35, #28.

Nothing from this work is left uncommitted. The only dirty file in the tree at
handoff time is `roadmap/roadmap.yaml`, carrying **another session's**
`supply-run-ngo` item (issue #52). That hunk was deliberately kept out of both
commits above and restored to the working copy afterwards - leave it for
whoever is writing it.

## What exists now

Engine (all under `arma_CWR/engine/Poseidon/`):

| File | What |
|---|---|
| `Game/Guerrilla/Traffic.hpp` / `.cpp` | the service: pure decision core + world layer + `Serialize` |
| `Game/Guerrilla/TrafficCommands.cpp` | `gmTraffic*` + `gmRoadNearest` / `gmRoadPath` / `gmRoadsNear` / `nearestRoads` |
| `Game/Commands/GameStateExtUi.cpp` | new global `NativeMoveIn(Person*, Transport*, GetInPosition)` |
| `Game/Guerrilla/ZoneRegistry.*` | `FactionRecord::civVehicles` + `FactionCivVehicles()`, plan-15 probed |
| `Game/Guerrilla/Undercover.*` | `ClassifyWeaponShow` is no longer file-static |
| `World/World.cpp`, `WorldInit.cpp`, `WorldImpl.cpp` | Simulate / InitMission / `GuerrillaTraffic` save block |

Behaviour: civilian cars CITY to CITY, occupier patrol hulls (`vehicles[0]`)
between occupier-held zones, rare war-scaled truck + escort convoys
(`vehicles[1]` + `vehicles[0]`). Player band `[300, 1500]` m, +300 m despawn
hysteresis, caps 3/1/1, one roll per 5 s pass rarest-kind-first. Commandeer:
player ahead in the lane (20 deg / 4 m) or weapon-in-hands aim (15 deg) inside
25 m -> Stop -> 2.5 s -> driver unassigned + `DoGetOut` + flees 150 m; hull
goes to the released table and is deleted only when the player is far and
nobody ever boarded it. Every civ driver carries the `driverKilled` killed-EH
expression, which `init.sqs` routes into `GM_fnCivKilledEH`.

Data: `civVehicles[]` on the CIV descriptor of `Guerrilla.Abel` and
`guerrilla_native.abel` (Skoda / SkodaBlue / SkodaRed / SkodaGreen / Rapid) and
of `Guerrilla.Sinai` / `Guerrilla.Lebanon80` (`LoBo_S1203`, `LoBo_Uaz3741`,
Skoda, Rapid - the two LoBo vans verified in `LoBoCars.pbo`, side=3 scope=2;
`LoBo_S1203CB` is the car bomb, deliberately excluded). `init.sqs` gained four
enqueue handlers plus the ledger expression and was synced byte-identical to
all eight copies (`test_mission_script_core` enforces this).

## Verification actually run (all green)

- `PoseidonTests.exe "[guerrilla]"` -> 140 cases / 2895 assertions.
- Trident `full_cwa`: `guerrilla_traffic_{ambient,commandeer,patrol}`, plus the
  regression pair `guerrilla_native_save_reload` (now carries a forced car's
  row across the cross-process reload) and `guerrilla_civ_ambient`.
- `--target Format`: the new files are clean; the target itself was already
  not baseline-clean before this work.
- Play check: ~9 min in `Missions\Guerrilla.Abel`, log shows a continuous
  spawn -> drive -> arrive/despawn cycle between Houdan, Dourdan, La Pessagne,
  Saint Louis and friends.

## The one thing worth remembering

An arcade ACMOVE waypoint, `AIGroup::Move`, and even a pure-script
`createVehicle` + `moveInDriver` + `group move` **never moved a freshly seated
vehicle crew**: the car sat with `unitReady false` and speed 0 for 90 s. A
`doMove` (`Command::Move` through `grp->IssueCommand(cmd, list)`, silent) drove
at 72 km/h immediately. Both group paths deliver through `SendCommand` (the
radio channel). Traffic therefore issues route legs, the commandeer Stop and
the flee Move as direct `IssueCommand` to the DRIVERS only. If some later
system needs waypoint-driven vehicle convoys, re-derive this first - do not
assume waypoints work.

Recorded durably in `guerrilla-mode/STATUS.md` (known edges), the roadmap
item's `log`, and the memory file `ambient-traffic-landed.md`.

## Known gaps left open on purpose

1. Patrol/convoy traffic feeds no `AlertMachine` input - a firefight with a
   road patrol raises no alert unless a zone garrison sees it. Optional
   follow-up: attribute `Traffic::IsTrafficGroup` patrol groups to the nearest
   occupier zone within `trafficRadius`.
2. The civ kill-ledger consumer (`civilians.sqs`) drops kills farther than
   `GM_CIV_EFFECT_R` (300 m) from the attributed town, so a deep-countryside
   road murder is written to `gmCivKilled` but stays unpunished.
3. A boarded released hull becomes an ordinary world object - persistent
   ownership is `cache-and-garage` (#28), out of scope here.
4. `guerrilla_native.abel`'s occupier `vehicles[]` is `{"Ural","BMP"}`, so
   patrols in that fixture are Urals. Fine for the test, worth a light hull if
   anyone tunes that descriptor.

## If you pick this up next

Likely next moves, in rough order of value: the AlertMachine attribution in
gap 1; a `trafficMaxLegs`-aware convoy escort that actually holds column
spacing over long hops; the `worldLocations` half of #33, which is all that
keeps `nearest-roads-command` from being fully closed as a title.
