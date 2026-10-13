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
| R | Liberated MILITARY zones are permanent in v1: nothing flips a resistance-held post back to the occupier, on or off screen. Towns are the exception since issue #79: a held town with occupier presence in it (live units, or an abstract counter-attack force, decision C) and no resistance presence drains `supportRate` per tick and reverts to occupier administration below `townRevertSupport` (default 30, under the flip line `supportFlip` 60, so a town just taken does not fall on the first occupier tick after the fighters walk out; section 3, row 2b). | Acknowledged; military recapture is a #38 follow-up |
| T | Towns have standing defenders (issue #79 B2, #72 first slice): every CITY the resistance does not hold is occupier-administered and keeps a `cityGarrison` reserve that the GarrisonCache materializes like a post's. Support cannot rise while any of it lives. | Landed (`ZoneRegistry::IsAdministeredTown`, keys `cityGarrison`, `townRetake`) |
| C | The occupier answers a town's rising (issue #79, the #77 counter-attack scoped to towns): a retake force drawn from the nearest occupier base's garrison comes whether or not the player is there. Inside the bubble it is a live squad; outside, an abstract force that travels, then holds the empty town (the retake drain does the rest) or rolls the odds against the fighters left in it. Each town already held makes the answer smaller and less likely and speeds the rise of the next town (section 9). Shakedown pairs no longer spawn in held towns (issue #79 D8). | Landed (`CounterAttack`, keys `counterAttack*`, `townRevertSupport`, `momentumSupport*`) |
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
group and companions, friendly hold squads, Legend bosses and guards, dealers,
town flags and a launched counter-attack squad (decision C). Both classes count against the budgets in section 6. "The
bubble" means `cacheRadius` (800 m) around the player.

## 3. Per-system policy

FREEZE: the record holds still off-screen. ABSTRACT: advances off-screen under
the section 2 rules. LIVE ONLY: exists only as entities near the player.

| # | System | Off-screen policy | Where |
|---|---|---|---|
| 1 | Capture meter (occupier OUTPOST/AIRFIELD/SEAPORT) | ABSTRACT (D1) when live resistance units are in `zoneArea` or `capture > 0`; otherwise FREEZE. The stored reserve counts as a defender. | `ZoneRegistry::EvaluateTick` |
| 2 | Town support (NEUTRAL or occupier CITY) | ABSTRACT (D1), gain only: +`supportRate` per tick with fighters present and no occupier presence; flips on the in-bubble rule. Occupier presence is a live unit in `zoneArea`, the town's stored garrison reserve, or a live body of its own garrison anywhere, so a remote town with a reserve and a squad is a frozen standoff. No remote intimidation. Script ledgers (resentment, civilian-kill deltas) run regardless of distance. | `ZoneRegistry::EvaluateTick`, `civilians.sqs` |
| 2b | Town retake (resistance-held CITY, `townRetake`) | ABSTRACT: the zone also ticks off-screen while occupier presence stands in it: live units, or an abstract counter-attack force holding the town (decision C, `ZoneTickInputs::abstractOcc`). Occupiers alone: -`supportRate` per tick, revert below `townRevertSupport` (owner back to its authored token, NEUTRAL for a resistance-authored town; garrison refilled to `garrisonMax`; `townLost` event). Fighters alone: +`supportRate` per tick up to 100 (no momentum on the hold). Both: frozen. | `ZoneRegistry::EvaluateTick`, `RevertTown`, `capture.sqs` |
| 2c | Counter-attack on a risen town (decision C) | Dispatch, delay and travel ABSTRACT and deterministic (rolls hashed from the flip). Town inside the bubble: LIVE squad (a base in the bubble marches its own garrison out; a stored reserve appears at the staging point after its road time), counted in the budgets, never cached back. Town outside: ABSTRACT arrival, then hold-and-drain (empty town) or one odds roll (fighters there); no entity created or deleted. | `CounterAttack`, `capture.sqs` |
| 3 | Flip side effects | Identical on and off screen: heat +40, income tap, hold squad, journal line, hint. | `EvaluateTick`, `capture.sqs` |
| 4 | Garrison reserve (posts and occupier-administered towns) | Count FREEZE (no attrition, no remote combat). Recovery ABSTRACT (D2): +1 per `garrisonRecoverInterval` up to `garrisonMax`, only while garrisonable (occupier post, or administered town not exempt), not materialized, the player alive and beyond the despawn edge, alert GREEN, `capture == 0` and no live resistance in the zone. Any disqualifier restarts the clock; a dead player freezes it. | `GarrisonCache::StepRecover` |
| 5 | Patrols | LIVE ONLY (traffic band); no off-screen position or schedule. | `Traffic` |
| 6 | Convoys | LIVE ONLY. A wiped convoy feeds the alert ledger (ambush floor, heat) and nothing else: no cargo, no logistics. | `Traffic`, `AlertMachine` |
| 7 | QRF | LIVE ONLY, free (no reserve debit), deleted on stand-down. Independent of the counter-attack (2c), which is fed by town flips, not alerts, and is debited from a base's garrison. | `qrf.sqs` |
| 8 | Supply / income | ABSTRACT every 600 s for the resistance. The occupier has no supply model. | `economy.sqs` |
| 9 | Heat | ABSTRACT decay on GREEN zones; raised only by live events. Off-screen zones are GREEN by construction, so heat cools everywhere the player is not. | `escalation.sqs`, `AlertMachine` |
| 10 | War level | ABSTRACT every 60 s from the share of zones held. | `escalation.sqs` |
| 11 | Alert state | Derived from live occupier knowledge; off-screen zones are GREEN, except the traffic-ambush floor within 1500 m of a wreck. | `AlertMachine` |
| 12 | Infrastructure | Town flags are always-present objects recoloured from the owner. Checkpoints, wire and berms do not exist yet; when they land they follow the materialization rule: persistent record, live in the bubble only, survivor write-back, regeneration only off-screen. | `TownFlags`; checkpoints are #35 |
| 13 | Civilians | LIVE ONLY; their ledgers (panic expiry, resentment) are ABSTRACT. | `civilians.sqs` |
| 14 | Civilian traffic | LIVE ONLY; entries persist in saves, remains clear at 1200 s or by distance. | `Traffic` |
| 15 | Enemy expansion | Absent. When built: ABSTRACT, never inside the bubble or in view, capped by `MaxZones` (64) and the war level. | #38 |
| 16 | Occupier recapture | Towns only: the counter-attack (row 2c) and the retake drain (row 2b). Liberated military zones stay permanent (decision R). | `CounterAttack`; posts are #38 |
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
| `garrisonMax` (per zone) | the zone's `garrison` | D2 ceiling; for a CITY also the retake refill |
| `cityGarrison` | 4 | occupier defenders of every administered town (whole number 0..24, repaired and logged otherwise); a CITY's own `garrison` overrides it; 0 = the pre-#79 free towns |
| `townRetake` | 1 | town retake on (0 = liberated towns are permanent and inert, the pre-#79 rule) |
| `townRevertSupport` | 30 | a held town reverts below this support, not below `supportFlip` (range 0..`supportFlip`, repaired and logged; `= supportFlip` is the pre-hysteresis rule) |
| `counterAttack` | 1 | 0 = no counter-attacks and no momentum (the whole pacing layer off) |
| `counterAttackSize` | 0.5 | first force, as a share of `groupSize` (12): 6 bodies |
| `counterAttackChance` | 1.0 | dispatch probability for the first town |
| `counterAttackDelayMin` / `Max` | 120 / 300 | seconds between the flip and the launch (drawn per flip) |
| `counterAttackSizeDecay` | 0.15 | force size x (1 - d) per town held before the flip |
| `counterAttackChanceDecay` | 0.15 | dispatch chance x (1 - d) per town held before the flip |
| `counterAttackSourceKeep` | 0.25 | share of a base's `garrisonMax` that never leaves it |
| `counterAttackSpeed` | 4 | abstract road speed (m/s) |
| `counterAttackRoadFactor` | 1.3 | road distance per straight-line metre |
| `counterAttackStageDistance` | 500 | a live squad from a stored reserve appears this far from the town, on its base's side, road-snapped |
| `counterAttackDefenderWeight` | 3 | one fighter in his town is worth this many attackers in the odds roll |
| `counterAttackRepelLoss` | 0.5 | share of a beaten force that does not come home |
| `counterAttackLiveTimeout` | 1200 | seconds before an unresolved live squad withdraws (only out of the player's sight) |
| `counterAttackMaxLive` | 2 | live counter-attack squads at once (more wait) |
| `momentumSupportStep` | 0.1 | support-rate multiplier step per town held |
| `momentumSupportCap` | 2.0 | ceiling of that multiplier |
| `simDiagInterval` | 60 | seconds between `[gmsim]` log lines (0 = off) |
| `budgetLiveBodies` | 130 | WARN above this many live AI bodies, all sides |
| `budgetSideGroups` | 40 | WARN above this many groups on one side (engine cap 63) |

The D2 clock is saved per zone (`recoverClock` on the zone row); a save from
before this change loads with a fresh clock. The zone row also carries
`garrisonExempt` (the new-game START TOWN headquarters is exempted from its
town garrison) and `flipDay` / `flipMinute` (the last flip to the resistance).
A save from before issue #79 keeps its towns' saved reserve of 0: they stay
free until D2 recovery regrows an administered town's reserve while the
player is away. `seenDay` / `seenMinute` now
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
| Abel (Malden) | None: the Airfield, Le Port Harbour and the Southeast Compound are authored (issue #76, Abel part). C1, C3, C4 and C6 pass. C5 fails: Goisse, La Pessagne, Vigny, La Riviere and Cancon are 3.1 to 4.6 km from any hub, Sainte Marie, Chapoi and Larche 2.4 to 2.9 km straight-line; #76 names no western hub. |
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
  contested, lastRemote, garrisonMax, recoverClock, garrisonLive,
  garrisonExempt, flipDay, flipMinute]` as of the last zone tick.
- `gmPlayerPresence` returns `[counts, undercover, exposed, weaponInHands,
  warriorOutfit, liberatedTowns]`: the issue #79 D1 rule sampled now (the
  player counts as resistance presence unless undercover with his cover
  intact: no living occupier group holds a compromised record of him, no
  weapon in hands, civilian outfit).
- `gmCounterAttacks` returns one row per pending or running counter-attack:
  `[town, source, phase, clock, strength, size, townsBefore, group]`, phase
  SCHEDULED / TRAVEL / HOLDING / LIVE; `gmCounterAttackTune [key, value]`
  overrides one tuning value at run time (tests).
- `gmSimDiag` returns `[zonesInBubble, zonesRemoteTicked, [liveOcc, liveRes,
  liveCiv, liveTotal], [grpOcc, grpRes, grpCiv, grpMaxSide],
  garrisonSpawnedZones, garrisonLiveBodies, [civ, parked, patrol, convoy],
  [gmAvg, gmP95, gmMax], [frameAvg, frameP95, frameMax], breachBits]`.
  Timings cover the profiler's last 256 frames.
- `gmSimDiagLog` writes one `[gmsim]` line now and returns the breach bits
  (1 = live bodies, 2 = groups on one side). The same line is written every
  `simDiagInterval` seconds.

Worst case at a hub, from the current defaults: garrisons in the bubble 24,
QRF 6, counter-attacks 12 (two squads of 6; a squad marched out of a base in
the bubble moves bodies, it adds none), traffic crews about 10, civilians 24,
shakedown 2, assailants 8 (about 86 bubble-bound), plus always-live Legends (up to 18), the player group (up to
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

Counter-attacks (decision C) on the shipped Abel template, Houdan:
`scripting/guerrilla_counter_attack_live` (player at the Camp: the Outpost
marches 5 of its 6 out within the delay window, they walk on the town, the
drain retakes it below 30 and the squad stays as its garrison) and
`scripting/guerrilla_counter_attack_offscreen` (player 2.5 km away: 5 leave
the Outpost's stored reserve, travel, hold the empty town and it reverts
with no entity created).

Budget soaks tour whole islands through the GUERRILLA menu with the shipped
templates: `ui/remaster/remaster_ud_offscreen_budget_abel` and
`ui/remaster/remaster_ud_offscreen_budget_sinai_lobo`. Results are in
[FULL-SCALE-SESSION.md](FULL-SCALE-SESSION.md).

## 8. Follow-ups

Owned elsewhere; this contract only fixes the rules they must follow.

- **#38 full mode:** author the section 5 hubs per template; fix Sinai
  seeding (C7) and cap dealers; scaffold detection of airfields, harbours and
  compounds plus a `playRegion[]` key; expansion under passivity on the
  abstract tick; occupier recapture of liberated MILITARY zones (towns have
  the counter-attack, decision C); caching friendly hold
  squads as a resistance reserve that still counts for D1; remote combat
  resolution for frozen standoffs.
- **#35 occupation director:** town security (#72) adopts the materialization
  rule (persistent reserve, live in the bubble only, survivor write-back, no
  remote intimidation); checkpoints as persistent records on road edges; QRF
  and Fireforce drawn from a regional pool with survivor write-back; convoys
  as carriers of D2 reinforcements; remote closures and reprisals on the
  abstract tick, deterministic and journaled.
- **#72:** the partial-spawn leak (a class that fails to spawn inside a
  garrison group is subtracted from the reserve; since issue #79 it is
  logged at WARN, not refunded, because a stored body beside a live
  garrison would block a town's support gate forever). Town defender
  placement is still `RandomNear` around the marker, and a YELLOW alert
  still sends every cached garrison group, town garrisons included, toward
  the last-known position.

## 9. Pacing: the counter-attack and momentum (decision C)

The brief (issue #79): a force answers the taking of a town, whether or not
the player is there; the first town is roughly even, slightly in the
resistance's favour; after that the resistance accelerates.

**Dispatch.** A town that rises (a CITY flipping to the resistance, the
start-town HQ excepted) is answered with probability
`chance(k) = counterAttackChance * (1 - counterAttackChanceDecay)^k`, where k
is the number of towns the resistance held before this flip. The force is
`size(k) = max(1, round(counterAttackSize * groupSize * (1 - counterAttackSizeDecay)^k))`
bodies, capped by what the nearest occupier base can spare:
`(stored reserve + live garrison) - floor(garrisonMax * counterAttackSourceKeep)`,
taken from the base's live garrison when the town is inside the bubble and
that garrison is materialized (the soldiers march out of the post), else from
its stored reserve (debited at launch). The nearest base with anything to
spare is chosen. No base can spare a man: no counter-attack. The force leaves after a delay
drawn from [`counterAttackDelayMin`, `counterAttackDelayMax`].

**The odds.** Off-screen, a force arriving at a town with fighters in it
rolls once: the town holds with probability `p = D*w / (A + D*w)`, D the
fighters the last zone tick counted in the town, A the attackers, w
`counterAttackDefenderWeight`. With the defaults the first town is the 60/40
target: three fighters (the starting squad) against six attackers hold with
`p = 9 / 15 = 0.6`. Live, the same squad fights for real; the 60/40 there is
a tuning target, not a measured rate (needs the human session). A beaten
force loses `ceil(A * counterAttackRepelLoss)`; the rest return to their base.
An empty town is not rolled for: the force holds it and the retake drain
reverts it after `(support - townRevertSupport) / supportRate` zone ticks.

**How later towns tilt.** With the defaults, three fighters left in the k-th
next town face:

| Towns held before (k) | Answered | Force | Town holds if answered | Town lost |
|---|---|---|---|---|
| 0 | 1.00 | 6 | 0.60 | 0.40 |
| 1 | 0.85 | 5 | 0.64 | 0.30 |
| 2 | 0.72 | 4 | 0.69 | 0.22 |
| 3 | 0.61 | 4 | 0.69 | 0.19 |
| 5 | 0.44 | 3 | 0.75 | 0.11 |

and every town held speeds the rise of the next administered town: its
support gain per tick is `supportRate * min(momentumSupportCap, 1 +
momentumSupportStep * townsHeld)` (1.1x with one town, 1.5x with five,
capped at 2x). A held town consolidates at the plain rate. Losing towns
loses the momentum, and wiping bases empties the reserves the answers are
drawn from, so both sides of the war feed the same ledger.

**Live squads and the bubble.** A live squad stays live until it resolves,
whichever way the player moves (the simpler rule: no cache transition, no
write-back mid-march); its bodies count against the budgets wherever it is,
and off-screen it works the town through the ordinary remote retake tick. It
resolves as: wiped out (repelled); the town reverts under it (it becomes the
town's garrison, the stored reserve tops it up to `garrisonMax`); or
`counterAttackLiveTimeout` passes and it is out of the player's sight
(withdraws, survivors home). A live spawn over the live-AI hard cap (130),
the side's group budget (40) or `counterAttackMaxLive` waits and retries
every 30 s, logged once; it is never dropped.

**Accidental retakes (issue #79 D8).** Any living occupier unit inside a held
town's `zoneArea`, with no fighters there, drains it. Shakedown pairs no
longer spawn in held towns (the organic picker checks the owner; the forced
debug hook does not). Traffic patrols passing through and Legend guards
parked inside a held town still count, on purpose: an occupier squad
standing in the street is occupation, whoever sent it. A patrol only drains
while it is inside the 150 m circle, so a passing column costs a few points
of support, not the town; a Legend guard post inside a held town will take
it unless fighters stay.

**Saves.** Pending and running counter-attacks are saved
(`GuerrillaCounterAttack` block: town and base by name, phase, clocks, the
roll seed, a live squad's group). Every roll is a hash of a seed fixed at the
flip, so a save and reload never changes an outcome.
