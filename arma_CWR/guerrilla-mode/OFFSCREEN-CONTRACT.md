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
| B | Budgets are pass/fail thresholds: 100 live AI typical, 130 hard cap, 40 groups per side, hub-scene frame time p95 <= 25 ms. Breaches log a WARN. With `occupierForces` on a born operation also counts against `forcesOpsMax` (2, read only from the occupier forces service, `gmForcesTune "opsMax"`) and its hulls against `forcesLiveHullsMax` (6). Per column: one infantry group, one crew group per hull, at most two hulls (carrier and escort), so 3 groups and 2 hulls; a birth that would exceed any budget waits. Lost hulls, crew groups and dead squads stay counted until the unseen cleanup deletes them. | Landed (`[gmsim]` line, keys `budgetLiveBodies`, `budgetSideGroups`, `forcesOpsMax`, `forcesLiveHullsMax`) |
| R | Liberated MILITARY zones are permanent in v1: nothing flips a resistance-held post back to the occupier, on or off screen. Towns are the exception since issue #79: a held town with occupier presence in it (live units, or an abstract counter-attack force, decision C) and no resistance presence drains `supportRate` per tick and reverts to occupier administration below `townRevertSupport` (default 30, under the flip line `supportFlip` 60, so a town just taken does not fall on the first occupier tick after the fighters walk out; section 3, row 2b). | Acknowledged; military recapture is a #38 follow-up |
| T | Towns have standing defenders (issue #79 B2, #72 first slice): every CITY the resistance does not hold is occupier-administered and keeps a `cityGarrison` reserve of whole bodies that the GarrisonCache materializes like a post's. Support cannot rise while any of it lives. A pending deposit (section 3, row 4) is not a defender. Only the player's CAPTURE presence raises support: not undercover, exposed, or a weapon in hands (issue #86; section 6, `gmPlayerPresence`). | Landed (`ZoneRegistry::IsAdministeredTown`, keys `cityGarrison`, `townRetake`) |
| C | The occupier answers a town's rising (issue #79, the #77 counter-attack scoped to towns): a retake force drawn from the nearest occupier base's garrison comes whether or not the player is there. Inside the bubble it is a live squad born unseen on its road and walking it to the town; outside, an abstract force that travels, then holds the empty town (the retake drain does the rest) or rolls the odds against the fighters left in it. The first town the resistance ever takes is answered 4 times in 10; from the second town on every town is answered, with no decay, by a force that grows with the war level, so two towns can be under attack at once (section 9). Holding towns gives the resistance no bonus. The occupier census has no owner or target filter: any living occupier unit in a held town counts as occupier presence there, a counter-attack column passing through on its way elsewhere included (OCCUPIER-FORCES.md D5). The town the player's headquarters stands in is never a counter-attack target (N4a). Shakedown pairs no longer spawn in held towns (issue #79 D8). An occupier forces pool, a standing retake director and reinforcement waves are being designed separately. | Landed (`CounterAttack`, keys `counterAttack*`, `townRevertSupport`) |
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
town flags and a counter-attack squad that has reached its town (decision C; a squad still on its road may return to the abstract road unseen). Both classes count against the budgets in section 6. "The
bubble" means `cacheRadius` (800 m) around the player.

## 3. Per-system policy

FREEZE: the record holds still off-screen. ABSTRACT: advances off-screen under
the section 2 rules. LIVE ONLY: exists only as entities near the player.

| # | System | Off-screen policy | Where |
|---|---|---|---|
| 1 | Capture meter (occupier OUTPOST/AIRFIELD/SEAPORT), and its mirror on a resistance-held post | ABSTRACT (D1) when live resistance units are in `zoneArea` or `capture > 0`; otherwise FREEZE. The stored reserve counts as a defender, and so does a live body of the post's own garrison within `cacheRadius` (800 m) of it, wherever he stepped to fight (U4). The player raises the meter only with CAPTURE presence. **The base recapture (OCCUPIER-FORCES.md 7.2, D19; key `baseRetake`, which follows `occupierForces` unless authored, replacing decision R):** a resistance-held post (not CAMP, not CITY) carries the occupier's own meter `occCapture` (0..100, saved, never shown as the player's progress, no `captureStarted` event). Any occupier unit in `zoneArea`, or an abstract BASE or HQ_ASSAULT force holding the post, with no resistance HOLD presence (hold squad, recruits, the player under the HOLD rule) raises it by `captureRate` x min(crew, 3) per tick; both present freeze it; resistance alone decays it at `captureDecayDefended`, nobody at `captureDecayAbandoned`. The zone also ticks off-screen while occupiers stand in it or `occCapture > 0`. At 100 the post reverts: owner back to the authored occupier token, the garrison settled (the holding force's bodies first, else a pool-funded deposit, pending inside the deposit bubble, row 4), income 0, `baseLost` event. | `ZoneRegistry::EvaluateTick`, `RevertBase` |
| 2 | Town support (NEUTRAL or occupier CITY) | ABSTRACT (D1), gain only: +`supportRate` per tick with fighters present and no occupier presence; flips on the in-bubble rule. Occupier presence is a live unit in `zoneArea`, the town's stored garrison reserve, or a live body of its own garrison anywhere, so a remote town with a reserve and a squad is a frozen standoff. No remote intimidation. Script ledgers (resentment, civilian-kill deltas) run regardless of distance. | `ZoneRegistry::EvaluateTick`, `civilians.sqs` |
| 2b | Town retake (resistance-held CITY, `townRetake`) | ABSTRACT: the zone also ticks off-screen while occupier presence stands in it: live units, or an abstract counter-attack force holding the town (decision C, `ZoneTickInputs::abstractOcc`; an HQ_ASSAULT force holding at an HQ inside the town counts too, OCCUPIER-FORCES.md 8.1). The same remote rule now opens resistance-held posts for the base recapture (row 1). Occupiers alone: -`supportRate` per tick, revert below `townRevertSupport` (owner back to its authored token, NEUTRAL for a resistance-authored town; garrison refilled up to `garrisonMax`, a pending deposit while the player is within `cacheRadius` + 50 m, row 4; `townLost` event). Fighters alone: +`supportRate` per tick up to 100. Both: frozen. Here the player counts by his HOLD presence: in uniform he always holds, weapon out or not; in a civilian outfit only when exposed, armed, or with his weapon seen by an occupier observer (section 6). | `ZoneRegistry::EvaluateTick`, `RevertTown`, `capture.sqs` |
| 2c | Counter-attack on a risen town (decision C) | Dispatch, delay and travel ABSTRACT and deterministic (rolls hashed from the flip). Town inside the bubble: LIVE squad walking its road to the town (a base in the bubble marches its own garrison out; a stored reserve is born unseen on its road, section 9), counted in the budgets; a marching squad nobody can perceive returns to the abstract road (its hulls deleted crew first, every crewman out of sight too). A ground hull is born and deleted only where the headlight band passes at night (Traffic's lights-on bound, a roof-high ray), not the infantry band. A force held at the town edge because the player stands in the town counts toward `counterAttackBirthGiveUp` (key on), then is born at the town edge out of sight or cancelled unseen. A lost hull's crew group, an abandoned intact hull and a wiped squad's dead group are deleted only where nobody sees them (a wreck stays). Town outside: ABSTRACT arrival, then hold-and-drain (empty town) or one odds roll (fighters there, key off); no entity created or deleted. With `occupierForces` on, fighters present make the arrival LIVE at the town edge (OCCUPIER-FORCES.md 6.5). | `CounterAttack`, `capture.sqs` |
| 3 | Flip side effects | Identical on and off screen: heat +40, income tap, hold squad, journal line, hint. | `EvaluateTick`, `capture.sqs` |
| 4 | Garrison reserve (posts and occupier-administered towns) | Count FREEZE (no attrition, no remote combat). Whole bodies only (U3). Despawn ADDS the survivors to whatever reserve is still stored (U1). Recovery ABSTRACT (D2): +1 per `garrisonRecoverInterval` up to `garrisonMax`, only while garrisonable (occupier post, or administered town not exempt), not materialized, the player alive and beyond the despawn edge, alert GREEN, `capture == 0` and no live resistance in the zone. Any disqualifier restarts the clock; a dead player freezes it. **No pop-in (S5):** a reserve write while the player is within `cacheRadius` + 50 m of the zone (the revert refill, survivors returning to a base, the reserve left waiting beside a wiped garrison) is a PENDING deposit: it does not defend and does not materialize. It joins the reserve once the player is beyond that edge, or once every sampled spawn point of the zone (a 3 x 3 grid over the +-`zoneArea`/3 placement square) is hidden to Traffic's perception gate, and it is checked before D2 runs. If the occupier loses the zone first, the deposit goes to the occupier forces pool when `occupierForces` is on (OCCUPIER-FORCES.md 3.3, 5.5); with the key off it goes back to its origin zone's reserve while the occupier holds that zone, else it is discarded (one INFO line). Ordinary materialization at 800 m stays ungated, so a raised view distance can still show pop-in. | `GarrisonCache::StepRecover`, `GarrisonCache::Deposit` |
| 5 | Patrols | LIVE ONLY (traffic band); no off-screen position or schedule. | `Traffic` |
| 6 | Convoys | LIVE ONLY. A wiped convoy feeds the alert ledger (ambush floor, heat) and nothing else: no cargo, no logistics. | `Traffic`, `AlertMachine` |
| 7 | QRF and alarm answers | With `occupierForces` off (or the service sized off, or a legacy save: `gmLegacyQrf`), the script QRF: LIVE ONLY, free (no reserve debit), deleted on stand-down, independent of the counter-attack (2c). With the key on in a new campaign the script QRF launches nothing (D20); a zone the resistance does not hold that sits RED is answered by the director instead (OCCUPIER-FORCES.md 8.2, D4): one ALARM record at a time, checked at the 5 s sighting cadence in any posture, from another occupier base (never the zone under attack) or by helicopter insertion, its infantry debited from that base's stored reserve and its hull and crew from the pool, born under the counter-attack's unseen-birth rule and moving at real speed (row 2c), sized against the zone's sightings (minimum 4), the cheapest package when the pool is THIN and none when it is SPENT. A repelled answer rests the zone. An old save keeps its own script QRF (`legacySave`, OCCUPIER-FORCES.md 11). | `qrf.sqs`, `OccupierForces::AnswerAlarms`, `CounterAttack` |
| 8 | Supply / income | ABSTRACT every 600 s for the resistance. A panicked town, and a town inside its strike window (row 17, `gmZoneStruck`), pays half its Roubles and no manpower. The occupier has no supply model beyond the forces pool. | `economy.sqs` |
| 9 | Heat | ABSTRACT decay on GREEN zones; raised only by live events. Off-screen zones are GREEN by construction, so heat cools everywhere the player is not. | `escalation.sqs`, `AlertMachine` |
| 10 | War level | ABSTRACT every 60 s from the share of zones held. | `escalation.sqs` |
| 11 | Alert state | Derived from live occupier knowledge; off-screen zones are GREEN, except the traffic-ambush floor within 1500 m of a wreck. | `AlertMachine` |
| 12 | Infrastructure | Town flags are always-present objects recoloured from the owner. Checkpoints, wire and berms do not exist yet; when they land they follow the materialization rule: persistent record, live in the bubble only, survivor write-back, regeneration only off-screen. | `TownFlags`; checkpoints are #35 |
| 13 | Civilians | LIVE ONLY; their ledgers (panic expiry, resentment) are ABSTRACT. A `townStruck` event starts the struck town's panic like a civilian killing does, wherever the player is. | `civilians.sqs` |
| 14 | Civilian traffic | LIVE ONLY; entries persist in saves, remains clear at 1200 s or by distance. | `Traffic` |
| 15 | Enemy expansion | Absent. When built: ABSTRACT, never inside the bubble or in view, capped by `MaxZones` (64) and the war level. | #38 |
| 17 | Air strikes on towns | The effects are ABSTRACT and identical on and off screen (OCCUPIER-FORCES.md 7.3, D17, D21): when a strike reaches its target (`CAOpStruck`, live or abstract) the struck town loses 40 resistance support and 40 occupier standing (floors at 0), gains 15 heat, is stamped `struckUntil` = now + 1200 s (half income, row 8) and fires `townStruck` (panic, row 13); every other town within 2 km loses 10 of each. A strike on a base (the opener of a base retake or HQ assault) cuts the 2 km ring only. Nothing raises the values back: a struck town recovers only by the presence rules (D18). Bought only while the pool is STRONG, on an urgent held town with 3 or more fighters reported, or as the opener of a base retake or HQ assault; aimed at the reported positions or the town centre, never a hidden player position (D9). The aircraft themselves are LIVE ONLY. | `ZoneRegistry::ApplyStrike`, `OccupierForces`, `CounterAttack` |
| 16 | Occupier recapture | Towns: the counter-attack (row 2c) and the retake drain (row 2b). Posts: the base recapture (row 1), which replaces decision R wherever `baseRetake` is on (by default wherever `occupierForces` is on): any occupier unit that clears the hold squad and stands in a held post takes it back, and with the pool on the director sends BASE operations while STRONG and through push-out, and reinforcement waves strike the densest cluster of held zones (OCCUPIER-FORCES.md 6.3, 6.4, 7.1). With `baseRetake` off liberated posts stay permanent (the old decision R). The HQ falls only to an HQ_ASSAULT through its own meter at `HqPos` (9.1, D14). | `ZoneRegistry::RevertBase`, `CounterAttack`, `OccupierForces` |
| 18 | Occupier air: births, LZ, removal | Abstract air flies the straight line from its origin at 0.8 x the airframe's `maxSpeed` (OCCUPIER-FORCES.md 3.2): an occupier-held AIRFIELD (the order's `airOrigin`), else the off-map edge (`forcesOffMapEdge` N / S / E / W, default the edge nearest the target; the point lies 1500 m beyond the map, level with the target). Off-map costs x1.5 (6.5, D3) and is farther. **Birth:** an airframe is born AIRBORNE (100 m above ground for a helicopter, 250 m for a jet), never on the ground, at the first point the altitude-aware gate (`OccupierAir::ExposureAt`, the #53 camera, band and terrain ray at the birth altitude, the 800 m air audible floor) rates HIDDEN, searched back from the abstract progress point to the origin, then off-map 1500 to 5500 m out; never within `counterAttackBirthMinToTarget` (250 m) of the target. Off-map is rendered sea and not automatically unseen. No point passes: the birth waits and the chain runs (3.1: next base back for ground, the air re-plan of a funded ground operation into a helicopter insertion bought from the pool, then cancel unseen). **LZ:** 300 to 500 m from the target on the side away from the reported fighters (OccupierForces sightings, never the player's true position, D9), object-free, dry, gradient within the engine's landing limit; an invisible pad marks it. The landing may be in plain view (D20). A touchdown more than 100 m off the LZ is a failed insertion: the airframe flies home with its cargo, is removed unseen, and the operation goes back to abstract flight once (the airframe re-created at the next birth, no refund), then is cancelled unseen. No LZ, or no airframe: the operation goes on foot, its price refunded. **Unload:** the cargo (a second group) gets out and becomes the operation's live squad; the airframe departs home. **Removal:** an airframe is deleted only where `OccupierAir::UnseenAt` passes at its altitude for 2 s: past the band's bound in 3D from the camera (nothing is drawn there, so this holds even inside the view cone: a speck at altitude is unseen), or outside the view cone and terrain-hidden, never inside the air audible floor; `departTimeout` (900 s, past the band) is the backstop. The CREW is deleted FIRST (each man out of his seat first, so nobody lingers in the world's out-vehicle list), the hull one step later, the pad with the hull. Then its price is refunded and the crew go home as bodies (`CAOpHullRefund`); a destroyed airframe is lost (`CAOpHullLost`). A strike (row 17) is live only while its target is in the bubble (a real sortie for `forcesSortieTime`, limited reveal of sightings under 60 s at accuracy 0.5, resistance AI only); otherwise its whole flight is abstract. Live airframes count against `forcesLiveHullsMax`; an insertion is two groups, a sortie one. Saved: the record's air fields, airframes flying home and OccupierAir's records (hull, crew, cargo and pad refs resolve on the second pass; a pad missing after a load is re-created). A base on another landmass than the target (`RoadRoute::SameLandmass`; Malden's Southeast Compound is an island) is never a ground source but stays an AIR-ONLY source: the director quotes only a helicopter insertion from it (`PackageSource::airOnly`), and the birth chain re-plans as air at once when no other base shares the target's landmass. | `CounterAttack` (CounterAttackAir.cpp), `OccupierAir` |

**These rules apply to every mission.** U1 to U5 (rows 1, 2b and 4), the
"any enemy drains" census (decision C) and the HOLD and CAPTURE presence
predicates (section 6) carry no config key: they change every Guerrilla
mission, old saves included (OCCUPIER-FORCES.md, phase 1).
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
| `counterAttack` | 1 | 0 = no counter-attacks at all |
| `counterAttackFirstTownChance` | 0.4 | dispatch probability for the first town the resistance ever takes (k = 0); range 0..1 |
| `counterAttackChance` | 1.0 | dispatch probability for every later town (k >= 1), no decay; range 0..1 |
| `counterAttackSize` | 0.5 | the force, as a share of `groupSize` (12): 6 bodies at war level 1 |
| `counterAttackWarLevelStep` | 0.25 | force x max(1, 1 + step x (war level - 1)), capped at one group of 12; range 0..2, 0 = flat size |
| `counterAttackDelayMin` / `Max` | 120 / 300 | seconds between the flip and the launch (drawn per flip) |
| `counterAttackSourceKeep` | 0.25 | share of a base's `garrisonMax` that never leaves it |
| `counterAttackSpeed` | 4 | abstract road speed (m/s); the abstract leg is the whole road from the base to the town |
| `counterAttackRoadFactor` | 1.3 | road distance per straight-line metre |
| `counterAttackBirthMinToTarget` | 250 | no counter-attack force is ever born this close to its target town (m, range 0..2000) |
| `counterAttackBirthGiveUp` | 600 | seconds a force with no unseen birth point (and, with `occupierForces` on, no other base whose road passes the gate) waits before it turns back and its strength returns to the base; with `occupierForces` on it is cancelled unseen instead, and a force held at the town edge (or suspended holding it) by the player in the town is born at the edge unseen or cancelled unseen after the same wait (range 30..86400) |
| `counterAttackDefenderWeight` | 3 | one fighter in his town is worth this many attackers in the odds roll |
| `counterAttackRepelLoss` | 0.5 | share of a beaten force that does not come home |
| `counterAttackLiveTimeout` | 1200 | seconds an unresolved live squad stays in its town before it withdraws (only out of the player's sight); the road march before it gets its own allowance (road left at 1 m/s + 120 s) |
| `counterAttackMaxLive` | 2 | live counter-attack squads at once (more wait) |
| `simDiagInterval` | 60 | seconds between `[gmsim]` log lines (0 = off) |
| `budgetLiveBodies` | 130 | WARN above this many live AI bodies, all sides |
| `budgetSideGroups` | 40 | WARN above this many groups on one side (engine cap 63) |
| `forcesSortieTime` | 300 | seconds a live strike spends over its target (range 60..1800; row 18) |
| `forcesOffMapEdge` | nearest | the parent nation's map edge for off-map air: "N", "S", "E" or "W"; absent or anything else = the edge nearest the target (row 18) |

The D2 clock is saved per zone (`recoverClock` on the zone row); a save from
before this change loads with a fresh clock. The S5 pending deposit is saved
on the zone row too (`pending`, and `pendingOrigin` by zone name); a save
without them loads with nothing pending, and a fractional saved reserve loads
floored to whole bodies. The zone row also carries
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
  garrisonExempt, flipDay, flipMinute, pending]` as of the last zone tick
  (`guer` is the CAPTURE presence; `pending` is the live S5 deposit).
- `gmPlayerPresence` returns `[counts, undercover, exposed, weaponInHands,
  warriorOutfit, liberatedTowns, holds, weaponSeen]`, sampled now. `counts`
  is CAPTURE (issue #86): not undercover, exposed (a living occupier group
  holds a compromised record of him), or a weapon in hands; the outfit alone
  never counts. `holds` is HOLD (OCCUPIER-FORCES.md D11, D23): not
  undercover, or a uniform (weapon out or not); in a civilian outfit only
  exposed, armed, or `weaponSeen`. `weaponSeen` is true while a living
  occupier observer has rated him suspect or exposed because of a visible
  weapon within the last 5 s (a slung rifle seen from behind or at
  conversational range counts; `gmUndercoverWeaponSeen` gives the group
  count). `warriorOutfit` is informational. CAPTURE raises a town's support
  and a post's capture meter; HOLD blocks the drain of a held town.
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
QRF 6, counter-attacks 12 (two squads of 6 at war level 1, up to 24 from war
level 5; a squad marched out of a base in the bubble moves bodies, it adds
none), traffic crews about 10, civilians 24, shakedown 2, assailants 8 (about
86 bubble-bound, 98 at war level 5), plus always-live Legends (up to 18), the player group (up to
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
`scripting/guerrilla_counter_attack_live` (player at the Camp; a first town,
its 0.4 roll tuned to 1: the Outpost marches 5 of its 6 out within the delay
window, they walk on the town, the drain retakes it below 30 and the squad
stays as its garrison) and `scripting/guerrilla_counter_attack_offscreen`
(player 2.5 km away; one town already counted as taken, so the default
chances answer it for certain: 5 leave the Outpost's stored reserve, travel,
hold the empty town and it reverts with no entity created).

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

## 9. Pacing: the counter-attack (decision C)

The rule (issue #79, set by the user on 2026-10-12): the first town the
resistance takes may or may not draw an immediate answer. From the second
town on, the occupier answers every town it loses, so two towns can be under
attack at the same time and both can be retaken. The resistance gets no
bonus for holding towns: every town rises at the plain `supportRate`.

**Counting towns.** k is the number of towns the resistance had already
taken before this flip. It is the larger of two numbers: a saved counter that
every flip of a non-exempt CITY to the resistance raises by one (a town taken
a second time after it fell counts again), and the non-exempt towns the
resistance holds at the flip (so a mission that opens with a town in
resistance hands has no "first town"). The start-town HQ never counts and is
never a target.

**Dispatch.** The first town (k = 0) is answered with probability
`counterAttackFirstTownChance` (0.4): six times in ten nothing comes for it.
Every later town (k >= 1) is answered with `counterAttackChance` (1.0), with
no decay however many towns are held. The roll is a hash of the flip (town,
flip stamp, k), never the engine RNG. The force is
`min(12, max(1, round(counterAttackSize * groupSize * max(1, 1 + counterAttackWarLevelStep * (warLevel - 1)))))`
bodies: 6 at war level 1, 8 at 2, 9 at 3, 11 at 4 and a full group of 12
from war level 5. The war level only scales the force up, never down. The
force is then capped by what the nearest occupier base can spare:
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

**What the player should expect.** With the defaults, off-screen, for a town
with three fighters left in it, or left empty:

| | Town 1 (k = 0) | Town 2 (k = 1) | Town 3 (k = 2) |
|---|---|---|---|
| A force comes | 4 times in 10 | always | always |
| When | 2 to 5 min after the flip | 2 to 5 min after the flip | 2 to 5 min after the flip |
| Force planned, war level 1 / 3 / 5+ (a base's spare caps what leaves) | 6 / 9 / 12 | 6 / 9 / 12 | 6 / 9 / 12 |
| Three fighters stay, war level 1: town lost | 0.4 x 0.4 = 0.16 | 0.40 | 0.40 |
| Three fighters stay, war level 3: town lost | 0.4 x 0.5 = 0.20 | 0.50 | 0.50 |
| Left empty: town lost | 0.40 | 1.00 | 1.00 |

Each flip draws its own force, one record per town, so after town 2 falls
to the resistance the answers to town 1 (if it drew one) and town 2 can be
on the road together; `counterAttackMaxLive` (2) caps the live squads in the
bubble, and more wait. Wiping bases empties the reserves the answers are
drawn from: a base that cannot spare a man sends nobody.

A standing retake director, an occupier forces pool and reinforcement waves
live in OCCUPIER-FORCES.md (key `occupierForces`); with that key on the
director takes every flip from the second distinct town on, and the
dispatch above answers only while it is DORMANT.

**Posts change hands too (base recapture, replacing decision R).** With
`baseRetake` on (by default wherever `occupierForces` is), a held post is
retaken the way the player takes one, in mirror: occupier units in it with
no resistance HOLD presence fill its `occCapture` meter at `captureRate` x
min(crew, 3) per 3 s tick, so a patrol that kills the 3-man hold squad and
stays takes the post in about 18 s; an abstract BASE force holding it does
the same off-screen. A BASE operation that arrives while the hold squad
lives goes live (born at the 250 m bound, even off-screen) and fights it.
The retaken post settles the holding force's bodies as its garrison (else a
pool-funded deposit), closes its income tap and fires `baseLost`; taking it
back fires `captured` again ("Retaken" in the journal, a new hold squad only
if the old one is dead).

**Unseen births (the invariant, user rule of 2026-10-13).** A counter-attack
soldier is never created where the player can perceive him: every force walks
the road from its base to the town (cross-country only when the road is
longer than twice the straight distance plus 300 m). A base's live garrison
marches out (it already exists). A stored reserve travels abstractly over the whole road
(distance x `counterAttackRoadFactor` / `counterAttackSpeed`) and is born as a
live squad only when the town or its progress point enters the player's
bubble, at a road point that passes Traffic's issue #53 gate: never inside
Traffic's audible floor (`trafficMinSpawnDist`), hidden by the draw distance
or terrain, or by the view cone alone only within 300 m of its base, and never
within `counterAttackBirthMinToTarget` of the town. A force launched with the
town already in the bubble searches from the base toward the town; a force the
player walked up to searches back from its progress point toward the base,
and one already inside the bound stays abstract at the town edge. No such
point, with `occupierForces` on (director and legacy records alike): it is
re-sourced at once to the next occupier base, nearest the town first, that can
spare the whole force and whose road (the road A*, else the straight
cross-country line) yields a point passing the gate; the first base refunds,
the new one pays, and the force is born from it. Only when no base passes does
it wait (one WARN, retried every 30 s, its arrival clock stopped, doing
nothing to the town); after `counterAttackBirthGiveUp` it tries the re-source
once more and is then cancelled unseen, never turned back (OCCUPIER-FORCES.md
3.1, D6). With the key off it waits the same way and turns back after
`counterAttackBirthGiveUp`, as before. A
holding force is never born at the town centre: with the town in the bubble
it is suspended (no drain, no odds roll). A marching squad that is out of the
player's perception and bubble for 10 s returns to the abstract road at its
progress, survivors and clock carried over. A live squad is occupier presence
in whatever town it stands in: a column passing another held town on its road
drains that town too (OCCUPIER-FORCES.md D5). A force that turns back to a
base inside the player's bubble comes home as a pending deposit (section 3,
row 4).

**Live squads and the bubble.** A live squad in its town stays live until it
resolves, whichever way the player moves; only a squad still marching its
road returns to the abstract road when nobody can perceive it (the invariant
above). Its bodies count against the budgets wherever it is, and off-screen
it works the town through the ordinary remote retake tick. It
resolves as: wiped out (repelled); the town reverts under it (it becomes the
town's garrison and its bodies come off the revert refill, so reserve, pending
deposit and squad together make `garrisonMax`; a reserve the town already
held beyond that refill is kept, U2); or
`counterAttackLiveTimeout` passes and it is out of the player's sight
(withdraws, survivors home). A live birth over the live-AI hard cap (130),
the side's group budget (40) or `counterAttackMaxLive` waits and retries
every 30 s, logged once; it is never dropped.

**Accidental retakes (issue #79 D8, OCCUPIER-FORCES.md D5, D18).** Any living
occupier unit inside a held town's `zoneArea`, with no resistance HOLD
presence there, drains it, with no ownership filter. Shakedown pairs no
longer spawn in held towns (the organic picker checks the owner; the forced
debug hook does not). Traffic patrols passing through, a counter-attack
column bound for another town and Legend guards parked inside a held town all
count, on purpose: an occupier squad standing in the street is occupation,
whoever sent it. A patrol only drains while it is inside the 150 m circle, so
a passing column costs a few points of support, not the town, and the town
stays weakened until the player or his fighters return; a Legend guard post
inside a held town will take it unless fighters stay. The player in uniform
holds the town he stands in, weapon out or not; disguised, he holds it only
when exposed, armed, or with his weapon seen.

**Saves.** Pending and running counter-attacks are saved
(`GuerrillaCounterAttack` block: town and base by name, phase, clocks, the
roll seed, a live squad's group, and the towns-taken counter that sets k).
Every roll is a hash of a seed fixed at the flip, so a save and reload never
changes an outcome. A save without the counter loads with 0, and the towns
held at the next flip stand in for it.

**Seams.** `CounterAttack::Decide(tuning, town, k, context)` is the whole
dispatch decision (go or no-go, and the force size), and
`CounterAttack::LaunchFrom(registry, view, town, source, size, ...)` sends a
force of a chosen size from a chosen base through the ordinary launch,
travel and arrival code. A later occupier forces service hooks in there.
