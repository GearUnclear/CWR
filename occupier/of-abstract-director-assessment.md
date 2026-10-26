# Abstract director tokens vs the occupier forces system as built

Assessment of the owner's question: "maybe the director should control some
abstracted representation of troops, their locations and macro behavior, then
they spawn in when the player is close? would that make more sense than these
static spawns or be less overhead?"

Read-only, 2026-10-14, branch `occupier-forces` at `16259cc`. Paths are under
`D:\Arma_CWA\arma_CWR\`; `CA` = `engine/Poseidon/Game/Guerrilla/CounterAttack.cpp`,
`OF` = `OccupierForces.cpp`, `GC` = `GarrisonCache.cpp`, `ZR` = `ZoneRegistry.cpp`.

## Short answer

The occupier side already IS the abstract-token model the owner describes.
An operation is a record with a target, a route, a real-speed clock and a
progress point; it is born as live AI only when the player is within 800 m
of its destination or of its own progress point (`CA:1523-1538`), and a born
force that walks back out of the player's world is dematerialised again after
10 s unseen, survivors and progress carried over (`CA:2658-2741`). Garrisons
are integer reserves that materialise at 800 m and write survivors back at
850 m (`GC:267-290`, `GC:966-976`). At 5 km from the player an operation, a
garrison, a helicopter insertion and a strike all exist as numbers.

What is NOT abstract today, and is the only place the proposal would change
runtime cost: (1) the resistance hold squads the player leaves on captured
bases are always-live script groups (`core/scripts/capture.sqs:201-209`), so
an operation that reaches such a base is born live and fights a real AI fight
wherever it is (`CA:2400-2414`, `2487-2495`), by design ("never rolls",
contract row 2c); (2) patrols, convoys and civilian traffic are live-only
inside the traffic band with no off-screen schedule at all (contract rows 5,
6, 14); (3) Leftovers, hulls driving home and airframes flying home stay live
until deleted unseen (`CA:3050-3121`, `3360`, `CounterAttackAir.cpp:604`).

So the proposal is a refinement of the birth chain plus two missing pieces (an
abstract resolver for force-vs-hold-squad, and abstract patrol tokens), not a
different architecture. "These static spawns" is not what the code does.

## 1. What is abstract and what is live today

| Thing | State at 5 km from the player | Evidence |
|---|---|---|
| Garrison of an occupier post or administered town | Integer `ZoneRecord::garrison`, plus a `pending` deposit; no entities. Materialised when `playerDistSq <= cacheRadius^2` (800 m), despawned beyond 850 m, survivors added back (U1). D2 regrowth is a clock on the row. | `GC:267-290` (`Decide`, `StepRecover`), `GC:907-1003` (the 5 s pass), `GarrisonCache.hpp:136` |
| Pending deposits and the release grid | Numbers. A reserve write inside 850 m waits until the player leaves or every sampled spawn point (3 x 3 grid + hashed per-body points) rates hidden. | `GC:426-520` (`InDepositBubble`, `PendingSpawnHidden`, `FlushPending`), `GC:944-955` |
| The pool, bands, tokens, waves, standing | Pure numbers, refilled on the 60 s director tick. | `OF:773-932`, `OF:1058-1115` |
| Director intel | A sighting table filled from the `TargetList` of living occupier groups every 5 s (`OF:3697-3761`), plus hashed informant tips. At 5 km the only living occupier groups are born operations and traffic, so far-away intel is heat, flips, alert edges and tips. | `OF:3738-3756`, `OF:2782` |
| Operation in CATravel (RETAKE, BASE, HQ_ASSAULT, REINFORCE, ALARM) | A record: `clock` counts the real-speed travel time down, `Progress(r)` and `RoutePoint` give its position on a road route sampled every 50 m. No group exists. | `CA:2353-2368`, `CA:600-612`, `CA:1472-1521` |
| The birth gate | `MustGoLive`: destination within `cacheRadius` of the player, or the player within `cacheRadius` of the route progress point. Then `GoLive` searches the route backward for a point Traffic's gate rates hidden, never within 250 m of the target. | `CA:1523-1538`, `CA:1830-1901`, `CA:393` (`PickSpawnPoint`) |
| Return to abstract travel | A born force on its road, away from the player and the destination (> 850 m), bodies and hulls unseen for `VanishHold` 10 s, is removed and its progress fraction re-timed (`ReturnToRoad`). Not while the road ahead passes the player (L13) and never for an arrival birth. | `CA:2658-2741`, `CounterAttack.hpp:557` |
| Arrival at an EMPTY target | Abstract: the record enters `CAHolding` and counts as `abstractOcc` in the zone tick, which drains support or fills `occCapture`. | `CA:2416-2427`, `CA:985` (`AbstractPresence`), `ZR:2940-2976`, `ZR:3406-3410` |
| Arrival at a target with live defenders | LIVE at any distance: `ArriveLive` births the force at the last route point beyond the 250 m bound and the fight is real. Over budget it waits at the edge, retried every 30 s. | `CA:2400-2414`, `CA:2808-2849` |
| Live force at its target, unresolved | Live until wiped, until the zone reverts under it, or `counterAttackLiveTimeout` 1200 s, then removed only unseen. | `CA:2761-2777` |
| Columns (hulls, crews) | Born with the squad under the hull gate, deleted with it on a return to abstract travel; hulls driving home stay live until deleted unseen. | `CA:1908-1960`, `CA:3336-3450` |
| Leftovers | Live (dead groups, abandoned hulls, stragglers) until unseen; list has no cap. | `CA:3036-3121` |
| Air insertion | Abstract straight line from `airFrom` (`CA:1480-1491`); born airborne through the same `MustGoLive` gate (`CA:1871-1874`); live only from birth to removal unseen or `departTimeout` 900 s. | `CounterAttackAir.cpp:338`, `OccupierAir.cpp:1619-1738` |
| Air strike | Abstract clock; goes live only while `strikePos` is in the bubble; abstract it kills nobody and applies the support and standing effects once. | `CounterAttackAir.cpp:875-926` |
| Player's hold squads on captured bases | ALWAYS LIVE script groups (`GM_HOLD_GRPS`), spawned by `gmFactionSquad` at capture, never cached. Towns have no hold squad: "cities hold themselves (support)". | `capture.sqs:186-209` |
| Player's recruits | Live units in the player's groups (`createUnit`); manpower is a number. | `recruit.sqs:161-165` |
| Patrols, convoys, civilian traffic | LIVE ONLY inside the traffic band; nothing exists off-screen. | contract rows 5, 6, 14; `Traffic.cpp:768-817` |

## 2. The cost of the live model

Where live groups can exist far from the player, and what bounds them:

- `GoLive` refuses a birth when `LiveSquads() >= counterAttackMaxLive` (2),
  `BornOps() >= OpsCap()` (`forcesOpsMax` 2 + one per wave record),
  `LiveHulls() + new > forcesLiveHullsMax` (6), or the 130-body or 40-group
  budgets would be exceeded (`CA:1848-1870`, `CA:787-823`). Unborn records
  are not counted anywhere, so the number of operations in flight is bounded
  only by command tokens (2, one per 1800 s in STRONG, `OF:2271-2278`).
- `CounterAttack::Simulate` steps once per second (`CounterAttack.hpp:534`)
  and `Advance`s every record: for an abstract record that is a clock
  decrement and a `MustGoLive` distance check; for a live record it is an
  `Alive` census, `CheckHull`/`CheckEscort`, a `March` re-order every 30 s
  and, on the road, `BodiesUnseen`/`HullUnseen` ray tests every tick
  (`CA:2505-2513`, `2666-2683`). The `gm` frame phase budget is avg 0.5 ms,
  p95 1.5 ms (contract section 6).
- The engine cost of a live group is the usual AI: group think, `RoadNet::SearchPath`
  (gives up after 100 nodes, `RoadRoute.hpp:8`), vehicle physics, and the
  known traps: stopped hulls and standing soldiers lock RoadLinks so a Move
  behind a stopped car off-roads (memory, issue #87 audit); a man deleted
  while seated lingers in `_outVehicles` and is written into saves (row 23,
  `EngineOps::LeaveVehicle`); dead units stay group members and hold a side
  slot until the body is deleted (the Leftover list exists to pay this off).
- Estimate from the code: a Malden session can hold at most 2 live operation
  records at once, each at most 3 groups and 2 hulls (infantry, carrier crew,
  escort crew), so 6 groups and 4 hulls, plus hulls driving home and
  airframes flying home up to the 6-hull cap, plus an uncapped Leftover list.
  The far-away live load is therefore small by configuration, not by
  measurement, and the only way to get a far-away live fight at all is an
  arrival against a live hold squad or the HQ.

So "less overhead" is not available from abstracting travel: travel is already
free. The overhead that exists is the arrival fight at a distant held base,
the cleanup tail (Leftovers, home hulls), and the always-live hold squads
themselves (one 3-man group per held base, whether or not anyone attacks).

## 3. The owner's proposal, assessed

(a) What it buys over today:
- CPU: nothing on travel (already abstract). Something on arrivals: an
  abstract resolver would remove the far-away live fights and most of the
  Leftover tail they generate.
- Scale: nothing on operations (unborn ops are already unlimited; the token
  budget is the limiter by design, 9.2). Real gains only if patrols become
  abstract tokens too: Traffic has no off-screen position or schedule today,
  so a "patrol pattern" the player can meet en route, or that drains a held
  town by passing, exists only inside the band.
- Determinism and saves: already there (hashed `Roll01`, records saved with
  clocks and seeds, `CA:5034-5130`). No gain.
- Legibility: the abstract position exists (`Progress`, `RoutePoint`,
  `gmOccupierOps` eta). A map overlay could read it today; the proposal adds
  nothing here beyond what #98 (journal integration) already needs.

(b) What it costs:
- A resolver. With the key on the code never resolves a fight abstractly
  (contract row 2c, the "No abstract odds roll" sentence; the unit case
  `test_counter_attack.cpp:2012` pins it). The key-off `Fight`
  (`CA:1753-1792`, `HoldOddsLethal` with `counterAttackDefenderWeight` 3 and
  `repelLoss` 0.5) is the obvious seed, but it is one roll at arrival, no
  duration, no state a live fight could resume from.
- Hold squads become a reserve. They are script-owned always-live groups;
  the resolver needs their count as a number and the GarrisonCache needs to
  spawn them on approach. That is a resistance-side mirror of the occupier
  cache (`IsGarrisonable` is occupier-only, `GarrisonCache.hpp:222-225`) and
  a save field.
- Mid-fight walk-in. If a fight resolves over time rather than in one roll,
  a token in a half-resolved fight must materialise with losses already
  applied on both sides; one-roll resolution avoids this at the price of
  the player never finding a fight in progress he did not start.
- Aircraft: unchanged (already abstract flights, live only in the bubble).
- Rules unaffected: decision S (unseen birth), the 250 m bound, the watched
  route chain and S5 all gate WHERE a birth happens, not WHETHER travel is
  abstract. They survive untouched.
- Decisions that need re-deciding: OF D19 ("any enemy unit that clears the
  player's squad and stands in a held base takes it back") and contract row
  2c ("the fight is real ... even off-screen"); the owner interview settled
  these on 2026-10-13. Also decision T/D1's "known artifact" (a frozen
  standoff when the player leaves mid-fight) becomes a resolver case.
- Save/load: a hold reserve per held zone, a resolver seed per record; the
  records themselves already carry what they need.

(c) How much survives: nearly all of it. `MustGoLive` is the single gate
between abstract and live for ground and air (`CA:2259`, `CA:1871`). "Birth
when near" is already "birth when near AND at an unseen point", so there is
no step to take there. The pieces to add are a resolver branch beside
`ArriveLive` at `CA:2400-2414` for out-of-bubble arrivals, and a reserve for
hold squads. The 70 `[counterattack]` and 68 `[occupierforces]` unit cases
keep their subject.

(d) Verdict on the architecture question: the current design is the owner's
proposal for operations, garrisons and air. It is not the proposal for
patrols (live-only, no tokens) and it deliberately declines the resolver.

## 4. The resolver question

- Occupier force vs EMPTY held zone: already abstract, through presence.
  `abstractOcc` (every holding record, `CA:985`) is added to `occ` in the
  zone tick and drains support (town) or fills `occCapture` (post), remote
  tick allowed (`ZR:2940-2976`, `ZR:2955-2956` the gate). Serviceable as is.
- Occupier force vs the player's hold squad: no abstract path with the key
  on; the force is born live (`CA:2413`). Key off: `Fight` one-roll odds.
- Occupier force vs resistance patrol: no resistance patrols exist; recruits
  travel with the player.
- Where the player's side is abstract today: town support (a number), zone
  ownership, manpower and resources (script globals). Not abstract: hold
  squads on bases, recruits, companions, the HQ (a meter at `HqPos`, but its
  defenders are live units within 150 m, `GuerrillaBase::TickHqMeter`).
- So: every fight that involves the player's men needs live AI today; every
  "fight" against nobody is already a meter. A resolver is needed only for
  the hold-squad case, and `HoldOddsLethal` already prices it.

## 5. Precedents

- Arma 3 Antistasi: garrisons are per-marker arrays of unit classes, spawned
  by distance; an attack reaching an unspawned marker is resolved by count
  comparison. Same shape as GarrisonCache plus the resolver this code lacks.
- Arma 3 Dynamic Simulation: freezes far entities but keeps them in the
  world; it trades CPU for memory and never abstracts, the opposite choice.
- OFP 1985 campaign: no off-screen handling; every unit always simulated,
  so missions were sized to the engine. Not a model to copy.
- The original Guerrilla Mode script QRF (`qrf.sqs`): live-only, free,
  deleted on stand-down; still the legacy path (row 7). It is what the
  occupier forces layer replaced.

## 6. Recommendation

Keep the current model; it already is the abstract model where it matters.
Take an incremental path only if the Malden playtest shows the far-away
arrival fights or the hold squads cost something measurable.

Steps, cheapest first:
1. Instrument (cheap, testable now): add to `[gmsim]`/`gmSimDiag` a count of
   live occupier groups beyond 850 m by cause (arrival fight, leftover, home
   hull, airframe), and log every `CAOpArrived` with "abstract"/"live" and
   the player distance. No test breaks; one unit case for the counter.
2. Hold squads as a cached reserve: a resistance-side mirror of GarrisonCache
   (`holdReserve` on the zone row, spawned at 800 m, written back at 850 m).
   This alone removes a live group per held base and fixes the D1 standoff
   artifact. Breaks: `capture.sqs` hold-group bookkeeping (`GM_HOLD_GRPS`),
   trident `guerrilla_forces_base_retake`, `guerrilla_counter_attack_live`,
   the Showcase "alert" human case, and the contract's always-live list.
3. One-roll resolver for an out-of-bubble arrival against the hold reserve:
   reuse `Fight` under the key with `HoldOddsLethal`; survivors on both sides
   settle as reserves. Breaks: unit `test_counter_attack.cpp:2012` and
   `2068`, contract row 2c, OF D19 (owner re-decision required).
4. Abstract patrol tokens (Traffic schedule): a separate, larger piece; only
   if the playtest shows the player rarely meets forces en route.

Overhead, separately:
- Engineering time: keeping = 0. Steps 1 to 3 = days, mostly tests and doc
  reconciliation. A rewrite to a token engine = weeks and discards work that
  already does the job.
- Runtime cost: today's far-away cost is at most 2 live operations plus the
  cleanup tail; steps 2 and 3 cut it to zero for arrivals; step 4 adds a
  small abstract tick per patrol token.

What the full Malden playtest should measure before deciding:
- `[gmsim] gm=` avg/p95 and frame p95 with 2 born ops plus a wave in flight.
- How many operations arrive "abstract" vs "live", and at what player
  distance; how many are cancelled unseen or hit `birthGiveUp`.
- Whether far columns ever arrive (CAOpArrived vs CAOpCancelledUnseen per
  package), and the Leftover list size over a 2-hour session.
- How often the player meets a born force en route (CAOpBorn with
  `spawnReason`) versus how often it leaves his world again (the "leaves the
  player's world" line), which is the number that says whether abstract
  patrol tokens would add anything.
- Live occupier groups beyond 850 m over time (step 1's counter).
