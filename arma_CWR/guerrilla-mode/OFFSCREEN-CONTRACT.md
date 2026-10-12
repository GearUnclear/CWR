# Off-screen contract: island-scale simulation in Guerrilla Mode

Issue [#71](https://github.com/GearUnclear/CWR/issues/71). Decided 2026-10-11.
Target data: the full official Remaster (plus `@LoBo` for Sinai and
Lebanon80). This file is the reference for what the campaign does where the
player is not looking. The runbook that exercises it on real data is
[FULL-SCALE-SESSION.md](FULL-SCALE-SESSION.md).

Like the rest of this folder it is a living design document: when the code
and this file disagree, check the code, then fix whichever is wrong.

## 1. Decisions

| ID | Decision | Status |
|---|---|---|
| D1 | A zone outside the bubble keeps ticking while live resistance units stand in it, or while its capture meter is above 0. Same rules as in the bubble: troops against an empty post secure and flip it, troops against a stored reserve are a frozen standoff, a half-finished capture decays when nobody holds it. Towns gain support remotely but never lose it to remote intimidation. | Landed (`ZoneRegistry::EvaluateTick`, key `remoteTick`) |
| D2 | A distant, calm, untouched enemy post regains 1 soldier per 600 s, up to its authored strength. | Landed (`GarrisonCache::StepRecover`, keys `garrisonRecoverInterval`, `garrisonMax`) |
| B | Budgets are pass/fail thresholds: 100 live AI typical, 130 hard cap, 40 groups per side, hub-scene frame time p95 <= 25 ms. Breaches log a WARN. | Landed (`[gmsim]` line, keys `budgetLiveBodies`, `budgetSideGroups`) |
| R | Liberated zones are permanent in v1: nothing flips a resistance-held zone back to the occupier, on or off screen. | Acknowledged; recapture is a #38 follow-up |
| X | Deferred to #38: the Sinai dealer cap (Sinai seeds 37 towns and draws 26 dealers) and the Sinai town-seeding fix (`seedCities = 1` seeds labels, airbases and borders as towns). | Deferred |

## 2. Definitions

**Full-island terrain.** The whole WRP world is loaded, rendered and
traversable, and every authored or seeded zone record exists from mission
start regardless of distance. A template may declare a smaller *playable
region* (Sinai: south Sinai; Lebanon80: South Lebanon). Coverage targets
(section 5) apply inside that region. Full-island terrain says nothing about
occupation density or remote activity.

**Persistent campaign state.** Everything that survives a cross-process save
and reload and does not depend on where the player stands: zone records
(owner, garrison reserve, support, income, heat, capture, revealed, seen, D2
recovery clock), the faction and side selection, SQS globals (resources,
manpower, war level, ledgers, companion XP), the Journal, Legend, Stash, HQ and
Market registries, traffic entries and the ambush queue, and entity handles
through the world serializer. It changes only through live tactical events,
the abstract ticks below, or explicit script writers. Crossing a simulation
boundary never changes it: cache transitions conserve body counts exactly.

**Off-screen operational simulation (the abstract tick).** Deterministic,
entity-free rules that advance persistent state on a fixed clock for places
outside the player's bubble. They read records and the island-wide live
presence counts, never create or delete entities, never draw random numbers,
and give the same result whether or not a save and load happened in between.
Members: economy (600 s), heat decay (10 s), war level (60 s), the
resentment and panic ledgers, dealer respawn, remote zone meters (D1, 3 s)
and garrison recovery (D2, 600 s).

**Live tactical simulation.** Engine-simulated entities, in two classes.
*Bubble-bound* entities are created by caches around the player and
reconciled back to records only through defined write-backs: garrisons
(spawn <= 800 m, despawn > 850 m), civilians (900 / 1200 m), traffic (band
>= 1500 m, despawn edge band + 300 m), the QRF (on a RED zone), shakedown
pairs and assailants. *Always-live* entities are never cached: the player's
group and companions, friendly hold squads, Legend bosses and guards, dealers
and town flags. Both classes count against the budgets in section 6. "The
bubble" means `cacheRadius` (800 m) around the player.

## 3. Per-system policy

FREEZE: the record holds still off-screen. ABSTRACT: advances off-screen under
the section 2 rules. LIVE ONLY: exists only as entities near the player.

| # | System | Off-screen policy | Where |
|---|---|---|---|
| 1 | Capture meter (occupier OUTPOST/AIRFIELD/SEAPORT) | ABSTRACT (D1) when live resistance units are in `zoneArea` or `capture > 0`; otherwise FREEZE. The stored reserve counts as a defender. | `ZoneRegistry::EvaluateTick` |
| 2 | Town support (NEUTRAL or occupier CITY) | ABSTRACT (D1), gain only: +`supportRate` per tick with fighters present and no occupier presence; flips on the in-bubble rule. No remote intimidation. Script ledgers (resentment, civilian-kill deltas) run regardless of distance. | `ZoneRegistry::EvaluateTick`, `civilians.sqs` |
| 3 | Flip side effects | Identical on and off screen: heat +40, income tap, hold squad, journal line, hint. | `EvaluateTick`, `capture.sqs` |
| 4 | Garrison reserve | Count FREEZE (no attrition, no remote combat). Recovery ABSTRACT (D2): +1 per `garrisonRecoverInterval` up to `garrisonMax`, only while occupier-owned, not materialized, the player alive and beyond the despawn edge, alert GREEN, `capture == 0` and no live resistance in the zone. Any disqualifier restarts the clock; a dead player freezes it. | `GarrisonCache::StepRecover` |
| 5 | Patrols | LIVE ONLY (traffic band); no off-screen position or schedule. | `Traffic` |
| 6 | Convoys | LIVE ONLY. A wiped convoy feeds the alert ledger (ambush floor, heat) and nothing else: no cargo, no logistics. | `Traffic`, `AlertMachine` |
| 7 | QRF | LIVE ONLY, free (no reserve debit), deleted on stand-down. | `qrf.sqs` |
| 8 | Supply / income | ABSTRACT every 600 s for the resistance. The occupier has no supply model. | `economy.sqs` |
| 9 | Heat | ABSTRACT decay on GREEN zones; raised only by live events. Off-screen zones are GREEN by construction, so heat cools everywhere the player is not. | `escalation.sqs`, `AlertMachine` |
| 10 | War level | ABSTRACT every 60 s from the share of zones held. | `escalation.sqs` |
| 11 | Alert state | Derived from live occupier knowledge; off-screen zones are GREEN, except the traffic-ambush floor within 1500 m of a wreck. | `AlertMachine` |
| 12 | Infrastructure | Town flags are always-present objects recoloured from the owner. Checkpoints, wire and berms do not exist yet; when they land they follow the materialization rule: persistent record, live in the bubble only, survivor write-back, regeneration only off-screen. | `TownFlags`; checkpoints are #35 |
| 13 | Civilians | LIVE ONLY; their ledgers (panic expiry, resentment) are ABSTRACT. | `civilians.sqs` |
| 14 | Civilian traffic | LIVE ONLY; entries persist in saves, remains clear at 1200 s or by distance. | `Traffic` |
| 15 | Enemy expansion | Absent. When built: ABSTRACT, never inside the bubble or in view, capped by `MaxZones` (64) and the war level. | #38 |
| 16 | Occupier recapture | Absent: liberated zones are permanent in v1 (decision R). | #38 |
| 17 | Always-live actors | LIVE everywhere, counted in the budgets. | Market, LegendRegistry, hold squads |

**Known artifact of D1.** If the player leaves while his fighters are still
in a firefight at a post, the surviving occupiers are written back as the
reserve and the zone becomes a frozen standoff until the player returns and
the garrison rematerializes. Remote combat resolution is a #38 follow-up.

## 4. Configuration keys

All live in `class CfgGuerrillaZones` of a template's `description.ext`.

| Key | Default | Meaning |
|---|---|---|
| `remoteTick` | 1 | D1 on (0 restores the old freeze outside `cacheRadius`) |
| `garrisonRecoverInterval` | 600 | D2 seconds per recovered body (0 = off) |
| `garrisonMax` (per zone) | the zone's `garrison` | D2 ceiling |
| `simDiagInterval` | 60 | seconds between `[gmsim]` log lines (0 = off) |
| `budgetLiveBodies` | 130 | WARN above this many live AI bodies, all sides |
| `budgetSideGroups` | 40 | WARN above this many groups on one side (engine cap 63) |

The D2 clock is saved per zone (`recoverClock` on the zone row); a save from
before this change loads with a fresh clock. `seenDay` / `seenMinute` now
mean "last time the player was within the bubble", not "last time the meters
moved".

## 5. Island coverage targets

Observable checks, not marker counts. All apply inside the playable region.

| ID | Criterion |
|---|---|
| C1 | Every military hub the map models in the region is a zone of the matching type: airfield as AIRFIELD, harbour as SEAPORT, garrisoned compound as OUTPOST. A map with none says so in its template header. |
| C2 | From the default Camp, a garrisoned occupier zone lies within 1.5 km and is reachable on foot in under 5 minutes. |
| C3 | At least one occupier zone pair within 3 km with a road path, so patrols are admissible from minute one (`gmTrafficDiag` at the Camp). |
| C4 | A progression ladder: at least 3 occupier military zones of rising garrison (about 6 / 8 / 12), the AIRFIELD (where one exists) the strongest. |
| C5 | Every CITY is within 3 km road distance of an occupier military zone at campaign start. |
| C6 | No zone centre in water or inside a walled compound; every centre reaches open terrain. |
| C7 | Seeded CITY zones are settlements inside the region (no labels, airbases, borders or islands). |

Candidate hubs found in the world data (anchors to verify against C6, not
final positions):

| Template | Hubs missing today |
|---|---|
| Abel (Malden) | Airfield (runway near [7922, 9590]), Le Port harbour (piers near [8476, 3754]), the fenced south-east military compound (near [9690, 3990]). The authored Outpost is the only occupier zone, so C3 and C4 fail. |
| Eden (Everon) | Airfield (near [4772, 10923]), a pier near [7392, 6835] to confirm as a harbour, and a real ladder in place of three generated garrison-6 outposts. |
| Sinai | Wadi airstrip next to WadiCheckpoint, the Egyptian Airbase (currently seeded as a CITY, a C7 defect), Sharm and El Tor harbours, the El Tor garrison. |
| Lebanon80 | No airfield or harbour inside the theatre by design (airpower is off-map, #35). Missing: the Tyre HQ compound, two border posts, the Mt Dov post. |

Authoring these is #38 work (see section 8).

## 6. Budgets and instruments

| Budget | Threshold | Instrument |
|---|---|---|
| Live AI bodies, typical | <= 100 | `gmSimDiag` element 2, `[gmsim] total=` |
| Live AI bodies, hard cap | <= 130 (WARN above) | same |
| Groups per side | <= 40 (WARN above; engine cap 63) | `gmSimDiag` element 3, `[gmsim] maxSide=` |
| Guerrilla service block ("gm" frame phase) | avg <= 0.5 ms, p95 <= 1.5 ms; spawn-burst frames are logged, not gated | `triPerfStats` `gm=`, `[gmsim] gm=avg/p95/max` |
| Frame time at a hub scene, 1080p, default view distance | p95 <= 25 ms (pass); p95 > 33.3 ms is a fail | dev panel Profile tab, `[gmsim] frame=` |

Instruments:

- `FrameProfiler` phase `gm` times the Guerrilla service block in
  `World::Simulate` (zone, garrison, flags, traffic, assailants, stashes, HQ,
  market, legends). `triPerfStats` and the dev Profile tab list it.
- `gmZoneSim <i>` returns `[inBubble, guer, occ, seenDay, seenMinute,
  contested, lastRemote, garrisonMax, recoverClock]` as of the last zone tick.
- `gmSimDiag` returns `[zonesInBubble, zonesRemoteTicked, [liveOcc, liveRes,
  liveCiv, liveTotal], [grpOcc, grpRes, grpCiv, grpMaxSide],
  garrisonSpawnedZones, garrisonLiveBodies, [civ, parked, patrol, convoy],
  [gmAvg, gmP95, gmMax], [frameAvg, frameP95, frameMax], breachBits]`.
  Timings cover the profiler's last 256 frames.
- `gmSimDiagLog` writes one `[gmsim]` line now and returns the breach bits
  (1 = live bodies, 2 = groups on one side). The same line is written every
  `simDiagInterval` seconds.

Worst case at a hub, from the current defaults: garrisons in the bubble 24,
QRF 6, traffic crews about 10, civilians 24, shakedown 2, assailants 8 (about
74 bubble-bound), plus always-live Legends (up to 18), the player group (up to
12), 3 per captured military zone in hold squads, and dealers. Sinai's 26
dealers are the known outlier (decision X).

## 7. Observable scenarios

Each scenario has an automated Trident test on the full Remaster lane
(`packages/RemasterUD`, tags `remaster_ud`, `offscreen`) using the fixture
mission `tests/integration/missions/guerrilla_offscreen.abel` (the native
parity mission plus a second occupier post at Malden's south-east compound,
D2 at 15 s and a 30 s ambush window). The "far point" is Malden's airfield
runway, 3.3 km from the Outpost.

| Scenario | Must persist | May change without the player | Test |
|---|---|---|---|
| (a) Leave friendly troops at a distant zone | The zone's intel stamp; a stored reserve; owner and capture during a standoff; support of a town where only occupiers stand | The capture meter climbs and the zone flips with the usual side effects; a town with an organizer gains support and rises; an unheld half-capture decays | `scripting/guerrilla_offscreen_capture` |
| (b) Wait in an untouched region | Every owner, support and capture; the war level; no garrison materializes; a full reserve never grows | A bled reserve regains 1 body per interval up to its authored strength; heat cools; income pays | `scripting/guerrilla_offscreen_idle` |
| (c) Interrupt a convoy, then leave and return | Reserves (the QRF is free, a convoy carries nothing); owners; no duplicate convoy on return | The attributed zone goes RED then back to GREEN; the QRF launches and stands down; heat rises then cools | `scripting/guerrilla_offscreen_convoy` |
| (d) Cross simulation boundaries and save/reload | A bled garrison stays bled across the bubble edge and across near and far saves reloaded in fresh processes; a distant standoff survives | Nothing, except D2 steps (kept out of the test's windows) | `scripting/guerrilla_offscreen_save_reload.seq` |

Budget soaks tour whole islands through the GUERRILLA menu with the shipped
templates: `ui/remaster/remaster_ud_offscreen_budget_abel` and
`ui/remaster/remaster_ud_offscreen_budget_sinai_lobo`. Results are in
[FULL-SCALE-SESSION.md](FULL-SCALE-SESSION.md).

## 8. Follow-ups

Owned elsewhere; this contract only fixes the rules they must follow.

- **#38 full mode:** author the section 5 hubs per template; fix Sinai
  seeding (C7) and cap dealers; scaffold detection of airfields, harbours and
  compounds plus a `playRegion[]` key; expansion under passivity on the
  abstract tick; occupier recapture of liberated zones; caching friendly hold
  squads as a resistance reserve that still counts for D1; remote combat
  resolution for frozen standoffs.
- **#35 occupation director:** town security (#72) adopts the materialization
  rule (persistent reserve, live in the bubble only, survivor write-back, no
  remote intimidation); checkpoints as persistent records on road edges; QRF
  and Fireforce drawn from a regional pool with survivor write-back; convoys
  as carriers of D2 reinforcements; remote closures and reprisals on the
  abstract tick, deterministic and journaled.
- **#72:** the partial-spawn leak (a class that fails to spawn inside a
  garrison group is silently subtracted from the reserve).
