# Off-screen contract: island-scale simulation in Guerrilla Mode

Issue [#71](https://github.com/GearUnclear/CWR/issues/71). Decided 2026-10-11;
rewritten for the occupier forces layer (OCCUPIER-FORCES.md, phase 7),
drafted 2026-10-13, reconciled 2026-10-14 against the committed phases 4
and 5 and the air spike (`c5cfa44`), and swapped in on 2026-10-14 with
phase 6 (`bcca80e`) folded in. Target data: the full official Remaster (plus `@LoBo`
for Sinai and Lebanon80). This file is the reference for what the campaign
does where the player is not looking. The runbook that exercises it on real
data is [FULL-SCALE-SESSION.md](FULL-SCALE-SESSION.md); the design behind the
occupier's side of it is [OCCUPIER-FORCES.md](OCCUPIER-FORCES.md).

Like the rest of this folder it is a living design document: when the code
and this file disagree, check the code, then fix whichever is wrong.

**Reading this file.**
- Decisions of OCCUPIER-FORCES.md are cited as OF D1 to OF D24, to keep them
  apart from this file's own D1 and D2.
- Every number carries its source: [16] the knob table, OCCUPIER-FORCES.md
  section 16; [code] the committed code on branch `occupier-forces` (read
  at `c5cfa44`; the places phase 6 changed re-read at `bcca80e`); [commit
  X] a behaviour stated in commit X's message (c825a5f 4B, 6e1ee27 4A,
  863b8aa and 37e24ba phase 5, c5cfa44 the air spike, bcca80e phase 6) and
  matching the committed contract rows, not re-read line by line in this
  pass; [air] AIR-SPIKE.md, its "Phase 6 proper (6A)" section included;
  [contract] the contract this file replaced. The working-tree tag [wt] of
  the first draft is retired: every such rule is now committed and
  retagged.
- Phases 1 to 6 have landed. The [phase 6] and [phase 6, pending] tags of
  the draft are retired: alarm answers, town strikes and air in
  `CounterAttack` and the director landed in `bcca80e`.
  [phase 7, landing with this commit] marks the new default of the key
  `occupierForces` (decision F, section 10.4).
- "(doc says X)" marks a place where OCCUPIER-FORCES.md and the code differ;
  the row states what the code does, and the code wins. Section 13 lists
  every such place with its resolution and the plan-doc line phase 7 edits.

## 1. Decisions

| ID | Decision | Status |
|---|---|---|
| D1 | A zone outside the bubble keeps ticking while live resistance units stand in it, or while its capture meter is above 0. Same rules as in the bubble: troops against an empty post secure and flip it, troops against a stored reserve are a frozen standoff, a half-finished capture decays when nobody holds it. Towns gain support remotely but never lose it to remote intimidation. A resistance-held town or post also ticks off-screen while occupier presence stands in it, or (a post) while its `occCapture` is above 0 (rows 1b, 2b). | Landed (`ZoneRegistry::EvaluateTick`, key `remoteTick`; the post half in `6e1ee27`) |
| D2 | A distant, calm, untouched enemy post regains 1 soldier per 600 s [contract], up to its authored strength. With `occupierForces` on every regrown body is drawn from the pool (`OccupierForces::Draw`): an empty pool grants nothing and the zone's clock is clamped, so no owed bodies are banked. | Landed (`GarrisonCache::StepRecover`, `ApplyRecover`, keys `garrisonRecoverInterval`, `garrisonMax`) |
| B | Budgets are pass/fail thresholds: 100 live AI typical, 130 hard cap, 40 groups per side, hub-scene frame time p95 <= 25 ms [contract]. Breaches log a WARN. Occupier operations add a command-token budget, a born-operation cap (`forcesOpsMax` 2, `forcesOpsMaxStrong` 3 while the pool is STRONG [2026-10-15], read only from the occupier forces service, `gmForcesTune "opsMax"` / `"opsMaxStrong"`; each living wave record raises it by one) and a live-hull cap (`forcesLiveHullsMax` 6). Per column: one infantry group and one crew group per hull, at most two hulls (carrier and escort), so 3 groups and 2 hulls; a birth that would exceed any budget waits. Lost hulls and crew groups stay counted while a member lives; a dead squad leaves the group budget at once and shows as `[gmsim] emptyGroups=`, and the unseen cleanup still deletes it (section 9.6) [commit 37e24ba; round 3.5]. Section 6. | Landed (`[gmsim]` line, keys `budgetLiveBodies`, `budgetSideGroups`; `forcesTokensMax`, `forcesOpsMax`, `forcesLiveHullsMax`) |
| R | **Liberated zones change hands both ways.** A held town with occupier presence and no resistance HOLD presence drains `supportRate` per tick and reverts below `townRevertSupport` (row 2b). A held post (not CAMP, not CITY) with occupier presence and no resistance HOLD presence fills the occupier's own meter `occCapture` and reverts at 100 (row 1b, OF D19). Any occupier unit counts, deliberate or passing (OF D5, OF D19). The player's HQ is not a zone and falls only to a deliberate assault (decision H). *Superseded:* until phase 4, liberated MILITARY zones were permanent ("nothing flips a resistance-held post back"); with `baseRetake = 0` they still are. | Towns landed (#79); posts landed (`6e1ee27`, `ZoneRegistry::RevertBase`, key `baseRetake`) |
| T | Towns have standing defenders (issue #79 B2, #72 first slice): every CITY the resistance does not hold is occupier-administered and keeps a `cityGarrison` reserve of whole bodies that the GarrisonCache materializes like a post's. Support cannot rise while any of it lives. A pending deposit (decision S5) is not a defender. Only the player's CAPTURE presence raises support (decision P). | Landed (`ZoneRegistry::IsAdministeredTown`, keys `cityGarrison`, `townRetake`) |
| C | **The occupier answers from a pool, not one force per town.** A finite theatre pool, counted in soldier-equivalents, pays for every occupier body that is not already standing somewhere: garrison regrowth, the refill of a retaken town or post, and every operation (decision F, section 10). The first town the resistance takes is answered by the old dispatch, 4 times in 10 [contract]. From the second distinct town on (OF D7 invariant W, `forcesWakeTowns` 2 [16]) a director answers instead: it sees only what its men saw (OF D9), spends 0 to 2 command tokens [16] on the hottest held towns, on held posts and the HQ while STRONG, and on quiet held zones through push-out (section 9.2). Three reinforcement waves refill the pool when control collapses and strike the densest cluster of held zones (decision W). An occupier zone at RED is answered by an ALARM operation (row 7), and air inserts troops and strikes towns (decision A). Every ground force walks or drives from a base on the target's landmass where its birth is unseen (decision S, the landmass rule of section 9.3); air flies in from an airfield or the map edge. Any occupier unit standing in a held town drains it, passing columns included (OF D5); the HQ's town is never the target of a town answer (N4a). Shakedown pairs do not spawn in held towns (issue #79 D8). *Superseded:* the #79 rule that answered every flip from k >= 1 with its own force (`counterAttackChance` 1.0, one record per town, no pool) runs only with `occupierForces = 0`, or while the director is DORMANT. | Pool and director landed (phase 3, `8f9482a`, `7a143ec`); posts, HQ and waves (phase 4, `6e1ee27`); every ground package and columns (phase 5, `863b8aa`, `37e24ba`); air, alarm answers, town strikes and the landmass rule (phase 6, `bcca80e`) |
| S | **Unseen birth.** No occupier force is ever created where the player can perceive it, and none within `counterAttackBirthMinToTarget` (250 m [16]) of its target. Section 9.3 is the hard rule. | Landed for ground forces (`450327e`, `7a143ec`, the chain completed in `6e1ee27`, the escort and the headlight gate in `37e24ba`, the landmass rule in `bcca80e`); air landed (spike GO `c5cfa44`, wired in `bcca80e`) |
| S5 | **No pop-in.** A reserve write within `cacheRadius` + 50 m (850 m [contract]) of the player is a pending deposit that neither defends nor materializes until he leaves or every sampled spawn point is hidden. Section 3, row 4 is the hard rule. | Landed (phase 1, `84fbbce`) |
| P | **Presence.** The player counts as resistance presence by two predicates: HOLD (blocks a drain, an occupier meter and the HQ meter) and CAPTURE (raises support and the player's own meter). Uniformed he always holds; disguised he holds only when exposed, armed, or with his weapon seen (OF D11, OF D23). Section 3, row 2b and section 6 (`gmPlayerPresence`). | Landed (phase 1) |
| H | **The HQ.** Only an HQ_ASSAULT the director deliberately sends can take the HQ (OF D14), through a meter at `HqPos` that is independent of zone ownership (OF D24). At 100 the HQ is destroyed with its weapon cache and every vehicle in its garage (OF D13, OF D22); the zone around it keeps its owner. The player may establish a new HQ anywhere, free. An HQ inside an enemy zone never suppresses that zone's defenders or census (OF D24). Section 11.1. | Landed (`c825a5f`, `GuerrillaBase::Destroy`, `TickHqMeter`) |
| L | **Lose.** The campaign is lost when the player has no HQ and holds no zone (no base, no town), after the campaign once had an HQ or a held zone (OF D10, OF D13). The end path mirrors the win with the `EMLoser` debriefing; saves stay available. Section 11.3. | Landed (`c825a5f`, `CampaignEnd` lose latch) |
| W | **Win and waves.** The win latches when every roster zone is held at once and nothing changes after it latches (OF D7). Three reinforcement waves at most [16] can retake posts, so the win needs every enemy base held at the same moment. Section 11.2. | Win landed (#79 B3); the freeze landed (phase 3, the lose freeze in `6e1ee27`); waves landed (`6e1ee27`) |
| F | **One currency, on by default.** The pool is the single occupier currency (section 10). Its key `occupierForces` defaults to 1 in the engine (`OccupierForcesTuning::KeyDefault`, the one default every reader of the key uses: the service, `CounterAttack` and the `baseRetake` fallback), so every Guerrilla template runs the layer whether or not it authors the key. A mission that is not a campaign (`campaignEnd = 0`: test fixtures and the reference slices) pins it off by authoring `occupierForces = 0` and runs the legacy rules of section 9.1. An old save keeps its script QRF through `legacySave` (`gmLegacyQrf`, row 7, section 10.4). | [phase 7, landing with this commit] |
| A | **Air.** Helicopters insert troops and leave; jets and attack helicopters strike reported fighters or a held town, never a hidden player position (OF D3, OF D17, OF D9). Air obeys decision S at its birth altitude, may land in plain view (OF D20), and is removed only where unseen; for an aircraft "unseen" also accepts a point beyond the perception band in 3D, even inside the view cone, with `departTimeout` (900 s) as the backstop. A strike on a town costs both sides support (OF D21). A base on another landmass than the target is an air-only source. Section 12, row 25. | Landed (`bcca80e`: `CounterAttackAir.cpp`, `OccupierAir`, the director's air buying; spike GO `c5cfa44`) |
| U | **Every mission.** U1 to U5 (rows 1, 2b and 4), the "any enemy drains" census (decision C) and the HOLD and CAPTURE predicates (decision P) carry no config key: they change every Guerrilla mission, old saves included (OCCUPIER-FORCES.md phase 1). | Landed |
| X | Deferred to #38: the Sinai dealer cap (Sinai seeds 37 towns and draws 26 dealers [contract]) and the Sinai town-seeding fix (`seedCities = 1` seeds labels, airbases and borders as towns). | Deferred |

## 2. Definitions

**Full-island terrain.** The whole WRP world is loaded, rendered and
traversable, and every authored or seeded zone record exists from mission
start regardless of distance. A template may declare a smaller *playable
region* (Sinai: south Sinai; Lebanon80: South Lebanon). Coverage targets
(section 5) apply inside that region. Full-island terrain says nothing about
occupation density or remote activity.

**Persistent campaign state.** Everything that survives a cross-process save
and reload and does not depend on where the player stands: zone records
(owner, garrison reserve, pending deposit, support, income, heat, capture,
`occCapture`, revealed, seen, D2 recovery clock, occupier `standing`), the
faction and side selection, SQS globals (resources, manpower, war level,
ledgers, companion XP), the Journal, Legend, Stash, HQ (with its destroyed
flag, epoch and meter) and Market registries, the occupier forces block
(pool, band, posture, tokens, sightings, operations, waves), counter-attack
and operation records with their hulls and airframes (`AirHomes` and
`OccupierAir`'s insertions and sorties), the campaign end latches, traffic
entries and the ambush queue, and entity handles through the world
serializer. It changes only through live tactical events, the abstract ticks
below, or explicit script writers. Crossing a simulation boundary never
changes it: cache transitions conserve body counts exactly, and the pool
ledger (section 10.3) balances.

**Off-screen operational simulation (the abstract tick).** Deterministic,
entity-free rules that advance persistent state on a fixed clock for places
outside the player's bubble. They read records and the island-wide live
presence counts, never create or delete entities, never draw engine random
numbers (every roll is hashed from a seed fixed at the event), and give the
same result whether or not a save and load happened in between. Members:
economy (600 s), heat decay (10 s), war level (60 s), the resentment and
panic ledgers, dealer respawn, remote zone meters (D1, 3 s), garrison
recovery (D2, 600 s) [contract]; the pool refill, control, the bands and the
director (`forcesDirectorInterval` 60 s [16]); abstract travel of unborn
forces; the occupier `standing` recovery (1 per 600 s toward 50 [16]) and
the informant roll; the wave triggers. The sighting scan (5 s,
`alertInterval` [16]) reads live occupier groups' knowledge; it is an input,
not a writer of world state.

**Live tactical simulation.** Engine-simulated entities, in two classes.
*Bubble-bound* entities are created by caches around the player and
reconciled back to records only through defined write-backs: garrisons
(spawn <= 800 m, despawn > 850 m), civilians (900 / 1200 m), traffic (band
>= 1500 m, despawn edge band + 300 m), the script QRF (on a RED zone,
legacy only, row 7), shakedown
pairs and assailants [contract]. *Always-live* entities are never cached:
the player's group and companions, friendly hold squads, Legend bosses and
guards, dealers, town flags, a force of any record kind that has reached its
target, the crews and hulls of a born vehicle package (carrier and escort)
until they are deleted unseen, an operation's airframe and its crew until
they are deleted unseen (row 25), and the entries of the Leftover list (lost
crew groups, abandoned hulls, dead squad groups) until the cleanup deletes
them unseen (section 9.6). A force still on its road may return to abstract travel unseen
(section 9.3). Both classes count against the budgets in section 6. "The
bubble" means `cacheRadius` (800 m [contract]) around the player; "the
deposit bubble" means `cacheRadius` + 50 m.

## 3. Per-system policy

FREEZE: the record holds still off-screen. ABSTRACT: advances off-screen under
the section 2 rules. LIVE ONLY: exists only as entities near the player.

| # | System | Off-screen policy | Where |
|---|---|---|---|
| 1 | Capture meter (occupier OUTPOST/AIRFIELD/SEAPORT) | ABSTRACT (D1) when live resistance units are in `zoneArea` or `capture > 0`; otherwise FREEZE. The stored reserve counts as a defender, and so does a live body of the post's own garrison within `cacheRadius` (800 m) of it, wherever he stepped to fight (U4). The player raises the meter only with CAPTURE presence. A pending deposit does not defend. | `ZoneRegistry::EvaluateTick` |
| 1b | Base recapture (resistance-held post, not CAMP, not CITY; key `baseRetake`, which follows `occupierForces` unless authored, an accepted deviation) | ABSTRACT while occupier presence stands in the post or `occCapture > 0`; otherwise FREEZE. `occCapture` (0..100, saved) is the occupier's own meter, never the player's `capture`: no "Securing" ticker, no `captureStarted`, `contested` or `captureLost` event. Occupier presence is any live occupier unit in `zoneArea` (OF D19), plus every abstract force of any kind holding the post and not inside the player's current bubble at the zone census (`CounterAttack::AbstractPresence`, "every kind counts" [code]; doc 7.2 names BASE and HQ_ASSAULT only). The census samples the player's bubble itself rather than the force's stored `suspended` flag, which lags one zone tick, so a force never drains a post in front of the player on the tick they arrive [code]. Resistance presence is HOLD: the hold squad, recruits, the player under the HOLD rule. Occupiers alone: + `captureRate` (6 [code], a per-zone `captureRate` override wins) x min(crew, `captureCrewCap` 3 [code]) per 3 s tick (`tickInterval` [code]), so a patrol that kills the 3-man hold squad and stays takes the post in about 18 s. Both: frozen. Resistance alone: - `captureDecayDefended` (10 [code]) per tick, an accepted deviation (doc 7.2 says resistance presence freezes it; the code wins); nobody: - `captureDecayAbandoned` (2 [code]) per tick. At 100 the post reverts (`RevertBase` [code]): owner back to its authored occupier token; the garrison settled, the holding force's bodies first up to the room with the surplus to the pool, else a pool-funded deposit, pending inside the deposit bubble (row 4); income 0; `lostCount` + 1; `baseLost` event. Once the campaign is won or lost `RevertBase` refuses [code]; the meter itself may still climb to 100 and sit there. Taking it back fires `captured` again ("Retaken" in the journal by `lostCount`; a new hold squad only if that zone's old one is dead, `capture.sqs` per-zone hold groups). With `baseRetake = 0` held posts stay permanent. | `ZoneRegistry::EvaluateTick`, `RevertBase`, `capture.sqs` |
| 2 | Town support (NEUTRAL or occupier CITY) | ABSTRACT (D1), gain only: + `supportRate` per tick with fighters present (CAPTURE for the player) and no occupier presence; flips on the in-bubble rule. Occupier presence is a live unit in `zoneArea`, the town's stored garrison reserve, or a live body of its own garrison anywhere, so a remote town with a reserve and a squad is a frozen standoff. No remote intimidation. Script ledgers (resentment, civilian-kill deltas) run regardless of distance. | `ZoneRegistry::EvaluateTick`, `civilians.sqs` |
| 2b | Town retake (resistance-held CITY, `townRetake`) | ABSTRACT: the zone also ticks off-screen while occupier presence stands in it. Occupier presence is any living occupier unit in `zoneArea` with no owner or target filter: a garrison that stepped in, a patrol, a column bound for another town, a Legend guard, the QRF (OF D5); plus an abstract force holding the town (`ZoneTickInputs::abstractOcc`). Occupiers alone: - `supportRate` per tick, revert below `townRevertSupport` (owner back to its authored token, NEUTRAL for a resistance-authored town; `townLost` event; garrison settled as in section 10.2). Fighters alone: + `supportRate` per tick up to 100. Both: frozen. **OF D18:** a town weakened by a passer-by stays weakened; support rises only with resistance presence, never passively. The player counts by HOLD (decision P). | `ZoneRegistry::EvaluateTick`, `RevertTown`, `capture.sqs` |
| 2c | Occupier operations (records of `CounterAttack`; kinds RETAKE, BASE, HQ_ASSAULT, REINFORCE, ALARM, STRIKE) | Planning, travel and arrival ABSTRACT and deterministic. Birth only under decision S (section 9.3). Born forces are LIVE and counted in every budget; a marching force nobody can perceive returns to abstract travel (its hulls deleted crew first, every crewman out of sight too, `HullUnseen` [commit 37e24ba]). A ground hull is born and deleted only where the headlight band passes at night (Traffic's lights-on bound, a roof-high ray), not the infantry band [commit 37e24ba]. A column is a carrier plus at most one escort (section 9.6). Arrival at an EMPTY target: the force holds it as abstract occupier presence and the zone tick drains or fills the meter. Arrival at a target with resistance defenders, with `occupierForces` on: the force goes LIVE (born under S at the town edge, even off-screen) and the fight is real; a hull package is born there with its carrier, its escort and their crews as on a route birth (the escort and an armed carrier support at the edge, a truck holds back; an insertion that arrived abstract is born on foot) [round 3.5, OCCUPIER-FORCES.md 6.5]; over budget, the hull gate (`forcesLiveHullsMax`) included, it waits at the target edge doing nothing to the target, retried every 30 s, never born on foot to fit. A force held at the edge because the player stands in the target counts toward `counterAttackBirthGiveUp`, then is born at the edge out of sight or cancelled unseen (the edge-hold give-up, review M2). A lost hull's crew group, an abandoned intact hull and a wiped squad's dead group go to the Leftover list and are deleted only where nobody sees them; a wreck stays (section 9.6). A live RETAKE, BASE or HQ_ASSAULT whose living infantry falls below a third of the strength it was born with while the target is still the resistance's breaks off (the lone survivor included); a withdrawal (break-off, `counterAttackLiveTimeout`, the HQ moved) settles at once as a `CAOpRepelled` with its reason, the survivors walking home as a withdrawing Leftover, and the hulls never leave before the infantry: seated, it rides home in its carrier; on foot, every hull holds until it has cleared 250 m of the target or 120 s have passed (section 9.6) [break-off, 2026-10-16]. A ground leg with no road route known is timed with the quote's `forcesRoadFactor` (`ForcePackages::RoadFactor`); `counterAttackRoadFactor` is a retired legacy key, ignored with one WARN. No abstract odds roll with the key on. With the key off, the legacy odds roll (section 9.1). A ground force is sourced only from a base on the target's landmass (section 9.3); a helicopter insertion, a strike and their airframes follow row 25 and section 12. | `CounterAttack` (air in `CounterAttackAir.cpp`), `OccupierForces` |
| 2d | HQ meter (`hqCapture`, at `HqPos`) | ABSTRACT. Only HQ_ASSAULT records count as attackers (`CounterAttack::HqAssaultPresence`, live or abstract, within 150 m of `HqPos` [commit c825a5f]); passing patrols, columns and QRFs never touch it, a patrol parked on the HQ included (OF D14). Rises by `captureRate` x min(assault units, `captureCrewCap` 3) per 3 s zone tick; resistance HOLD presence within 150 m (the player under the HOLD rule plus every live resistance unit) freezes it; with no assault units it decays at `captureDecayAbandoned` (2); there is no defended-decay path [code] (`GuerrillaBase::NextHqCapture`, `EngineHqHold`). At 100 the HQ is destroyed (section 11.1) and the HQ records settle on `hqLost`. The zone around the HQ follows its own rows. | `GuerrillaBase::TickHqMeter`, `NextHqCapture` |
| 3 | Flip side effects | Identical on and off screen: heat +40 [contract], income tap, hold squad, journal line, hint. A post retaken by the occupier closes its income tap and fires `baseLost`; a destroyed HQ fires `hqLost`. The director hears flips and owner changes as events (OF D9). | `EvaluateTick`, `capture.sqs`, `OccupierForces` |
| 4 | Garrison reserve (posts and occupier-administered towns) | Count FREEZE (no attrition, no remote combat). Whole bodies only (U3). Despawn ADDS the survivors to whatever reserve is still stored (U1). Recovery ABSTRACT (D2): + 1 per `garrisonRecoverInterval` up to `garrisonMax`, paid by the pool when it is on, only while garrisonable (occupier post, or administered town not exempt), not materialized, the player alive and beyond the despawn edge, alert GREEN, `capture == 0` and no live resistance in the zone. Any disqualifier restarts the clock; a dead player freezes it. **No pop-in (S5), a hard rule:** a reserve write while the player is within the deposit bubble (850 m) of the zone is a PENDING deposit: the revert refill, survivors returning to a base, a re-sourced or cancelled force's bodies going home, the reserve left waiting beside a wiped garrison. A pending deposit does not defend and does not materialize. It joins the reserve once the player is beyond that edge, or once every sampled point of the zone (a 3 x 3 grid over the +- `zoneArea`/3 placement square [contract], plus the spawn point of every body the next materialization would place: `GarrisonCache::SpawnPoint`, hashed from the zone name and the body's index, the very points `SpawnGarrison` uses, review L11) is hidden to Traffic's perception gate, and it is checked before D2 runs. A reserve write outside the zone tick (a revert, a wave's allocation) takes the player from the last zone tick; before the first tick since a load it asks the world for the player, and with no player known it is pending (`ZoneRegistry::AddReserveNow`, review L10). If the occupier loses the zone first, with `occupierForces` on the deposit goes to the pool (`OccupierForces::Clear` installs `GarrisonCache::SetPendingRefundHook`; the 5.5 equation holds across the change [commit 6e1ee27]); with the key off it goes back to its origin zone's reserve, up to that zone's room, while the occupier holds it, and the rest is discarded (one INFO line). Ordinary materialization at 800 m stays ungated, so a raised view distance can still show pop-in. Tests: `guerrilla_forces_no_popin`, `guerrilla_forces_wiped_release`, `guerrilla_counter_attack_watched_base`, `[guerrilla]` S5 cases. | `GarrisonCache::StepRecover`, `Deposit`, `AddReserve`, `RefundPending` |
| 5 | Patrols | LIVE ONLY (traffic band); no off-screen position or schedule. A patrol standing in a held town drains it (row 2b); in a held post it fills `occCapture` (row 1b). Traffic moves a seated man out of the hull before it deletes him (row 23). A Traffic vehicle never drives to a zone on another landmass than its origin (`PickDest`, section 9.3). | `Traffic` |
| 6 | Convoys | LIVE ONLY. A wiped convoy feeds the alert ledger (ambush floor, heat) and nothing else: no cargo, no logistics. | `Traffic`, `AlertMachine` |
| 7 | QRF and alarm answers | **Script QRF** (legacy): LIVE ONLY, free (no reserve debit, no pool debit), deleted on stand-down, independent of the counter-attack (row 2c). `qrf.sqs` fields it only while `gmLegacyQrf` is true: `occupierForces` off, the service sized off, or a legacy save (`legacySave`: a forces block older than format 6, or one derived from a save without the block) [commit bcca80e]. With the key on in a new campaign the script QRF launches nothing (OF D20). **Alarm answers** (OF D4, OCCUPIER-FORCES.md 8.2) [commit bcca80e]: a zone the resistance does not hold that sits RED is answered by the director with one ALARM operation (`CAKAlarm`) at a time, checked at the 5 s sighting cadence. It takes no command token and runs while DORMANT too (an accepted deviation: otherwise a key-on campaign has no answer before the second town); the cooldowns apply. Sources: every occupier base but the zone under attack (ground sources on the target's landmass only, section 9.3), or a helicopter insertion; its infantry is debited from that base's stored reserve, its hull and crew from the pool. Sized against the zone's sightings, minimum 4 bodies (`ForcePackages::AlarmMinBodies`) [code], always urgent except THIN, which takes the cheapest package; SPENT does not answer ("Starved" once). Born under decision S and moving at real speed (row 2c), it travels to `alarmPos` (the latest report) and emits `CAOpAlarmAnswered` on arrival. Live, it holds there until the zone is below RED with no resistance presence for 60 s (`AlarmQuietHold`) or the live timeout, then joins the garrison up to its room like a live top-up (the stored reserve, then the pending deposit, give way to the pool; the surplus splits off and walks home as stragglers, `CounterAttack::AdoptTopUp`); abstract, it tops the garrison up to its room at once and the surplus goes to the pool. A repelled answer rests the zone (`forcesTownCooldown`). | `qrf.sqs`, `OccupierForces::AnswerAlarms`, `CounterAttack` |
| 8 | Supply / income | ABSTRACT every 600 s for the resistance [contract]. A panicked town, and a town inside its strike window (row 21, `gmZoneStruck`: mission time < `struckUntil`), pays half its Roubles and no manpower [commit bcca80e]. The occupier's supply model is the pool (row 18). | `economy.sqs` |
| 9 | Heat | ABSTRACT decay on GREEN zones; raised only by live events, by a repelled operation (+15 [code] unless a RED edge already paid it), by a town strike (+15 in the struck town, row 21) and by the director's re-arm: a held zone whose rest ended with fighters still in the director's sighting table is held at `forcesRearmHeat` (40) until an operation goes for it, its reports decay or it changes hands (OCCUPIER-FORCES.md 6.3) [code, round 3.5]. Off-screen zones are GREEN by construction, so heat cools everywhere the player is not. Every resistance-held zone, towns and posts, decays at `gmHeatDecayHeld` (0.25 per 10 s [16]), so a flip (+40) is remembered for about 27 minutes (40 / 0.25 x 10 s). | `escalation.sqs`, `AlertMachine` |
| 10 | War level | ABSTRACT every 60 s from the share of zones held. The occupier-side readers (garrison and operation tiers, traffic classes, undercover detection, the QRF) read the occupier war level: the territory level plus `forcesWaveWarLevel` (1 [16]) per landed wave (`gmWarLevelOccupier` [commit 6e1ee27]). The resistance keeps the territory level. With the key off both are equal. | `escalation.sqs`, `OccupierForces::OccupierWarLevel` |
| 11 | Alert state | Derived from live occupier knowledge; off-screen zones are GREEN, except the traffic-ambush floor within 1500 m of a wreck [contract]. Alert edges on held towns latch the director's urgency (section 9.2); the director never reads `AlertMachine`'s player-position fallback (OF D9). | `AlertMachine` |
| 12 | Infrastructure | Town flags are always-present objects recoloured from the owner. Checkpoints, wire and berms do not exist yet; when they land they follow the materialization rule: persistent record, live in the bubble only, survivor write-back, regeneration only off-screen. | `TownFlags`; checkpoints are #35 |
| 13 | Civilians | LIVE ONLY; their ledgers (panic expiry, resentment) are ABSTRACT. A `townStruck` event (row 21) starts the struck town's panic like a civilian killing does (`GM_fnPanicStart`), wherever the player is [commit bcca80e]. | `civilians.sqs` |
| 14 | Civilian traffic | LIVE ONLY; entries persist in saves, remains clear at 1200 s [contract] or by distance. | `Traffic` |
| 15 | Enemy expansion | Absent. Push-out and waves are not expansion: they retake zones the resistance liberated and never create zones. When built: ABSTRACT, never inside the bubble or in view, capped by `MaxZones` (64 [contract]) and the war level. | #38 |
| 16 | Occupier recapture | Towns: row 2b, by any occupier presence, and director operations (section 9.2). Posts: row 1b, by any occupier presence, BASE operations while STRONG and through push-out (OF D16), and wave strikes. The HQ: only an HQ_ASSAULT through row 2d. Decision R. | `ZoneRegistry::RevertTown`, `RevertBase`, `CounterAttack`, `OccupierForces` |
| 17 | Always-live actors | LIVE everywhere, counted in the budgets. | Market, LegendRegistry, hold squads |
| 18 | Occupier pool | ABSTRACT. Refilled each director tick, spent at launch and on refills, credited by refunds and waves. Section 10. With `occupierForces = 0` every pool seam passes through (D2 regrows from nothing, refills come from nothing). | `OccupierForces` |
| 19 | The director | ABSTRACT, one step per `forcesDirectorInterval` (60 s [16]). Reads only flips, owner changes, heat, alert states, its sightings and its own operations' events (OF D9); never the true census, never the player's position. Launches at most one operation per step. Section 9.2. | `OccupierForces::Direct` |
| 20 | Reinforcement waves | ABSTRACT until their operations are born. Section 11.2. | `OccupierForces` [commit 6e1ee27] |
| 21 | Air strikes on towns | The effects are ABSTRACT and identical on and off screen (OCCUPIER-FORCES.md 7.3, OF D17, OF D21) [commit bcca80e]: when a strike reaches its target (`CAOpStruck`, sent once, live or abstract) the struck town loses 40 resistance support and 40 occupier standing (floors at 0), gains 15 heat, is stamped `struckUntil` = now + 1200 s (half income and no manpower, row 8) and fires `townStruck` (panic, row 13); every other town within 2 km loses 10 of each (`ZoneRegistry::ApplyStrike`). A strike on a base (the opener of a base retake or HQ assault) or on a position cuts the 2 km ring only. Nothing raises the values back: a struck town recovers only by the presence rules (OF D18), and a struck held town usually falls below `townRevertSupport` and reverts to the next occupier in it. Bought only while the pool is STRONG after the ground operation's debit and the strike's own (the airframe must leave the band STRONG: the attack helicopter when the jet would demote it, none when neither keeps it; the `gmForcesForce "strike"` lever is exempt) [code, round 3.5]: as an add-on right after a director ground launch on an URGENT held town with 3 or more fighters reported, outside its strike window; as the opener of a base retake or HQ assault with at least one defender reported in the director's sighting table, never on an empty target (launched just after the ground order under one band hold, so the aircraft arrives first) [code, round 3 C]; or alone on such a town when no ground package can go. Aimed at the centroid of the zone's reported positions (OF D9), or `HqPos` for an HQ with nothing reported there, never a hidden player position. The flight is abstract unless its target is in the bubble; the aircraft themselves are LIVE ONLY (row 25): live, the pass is real and the aircraft's own sensors pick its targets, civilians included; abstract, nobody is killed and the cost is the support and standing loss. Wave strikes buy no air opener (an accepted deviation). | `ZoneRegistry::ApplyStrike`, `OccupierForces`, `CounterAttack`, `civilians.sqs`, `economy.sqs` |
| 22 | Occupier standing (per CITY, 0..100, default 50 [16]; saved on the zone row; `gmZoneSet "standing"`) | ABSTRACT: recovers toward 50 at 1 per 600 s [16]. It drives informant tips (chance `standing / 200` at standing >= 50 [16]; a tip writes the town's TRUE resistance count, its position jittered 100 m, into the sighting table, an accepted deviation that 7.3 frames as D9-legal), garrison morale (skill x 0.8 below 25 [16]) and the refill factor (section 10.1). A town strike cuts it (row 21). | `OccupierForces` [commit 6e1ee27] |
| 23 | Removing a seated man (engine) | `::DeleteVehicle` on a man sitting in a vehicle only flags him: he lingers in `World::_outVehicles` and is written into saves. Every occupier removal moves a seated man out of the vehicle first, then deletes him (`EngineOps` [commit 37e24ba]). Traffic does the same on every seated-deletion path [commit 2524c3a], and so does `OccupierAir` for an airframe's crew and any cargo still seated [commit bcca80e]. | `EngineOps`, `Traffic`, `OccupierAir` |
| 24 | Leftover cleanup | LIVE until deleted unseen; the list is saved (`Leftovers`). Section 9.6. | `CounterAttack` [commit 37e24ba] |
| 25 | Occupier air: births, LZ, removal | Abstract air flies the straight line from its origin at 0.8 x the airframe's `maxSpeed` (OCCUPIER-FORCES.md 3.2): an occupier-held AIRFIELD (the order's `airOrigin`, the one nearest the target), else the off-map edge (`forcesOffMapEdge` N / S / E / W, default the edge nearest the target; the point lies 1500 m beyond the map, level with the target), fixed at launch. Off-map costs x1.5 (6.5, OF D3) and is farther. **Birth:** an airframe is born AIRBORNE (100 m above ground for a helicopter, 250 m for a jet), never on the ground, at the first point the altitude-aware gate (`OccupierAir::ExposureAt`, the #53 camera, band and terrain ray at the birth altitude, the 800 m air audible floor) rates HIDDEN, searched back from the abstract progress point to the origin, then off-map 1500 to 5500 m out; never within `counterAttackBirthMinToTarget` (250 m) of the target. Off-map is rendered sea and not automatically unseen. No point passes: the birth waits and the chain of section 9.3 runs. **LZ:** 300 to 500 m from the target on the side away from the reported fighters (OccupierForces sightings, never the player's true position, OF D9), object-free, dry, gradient within the engine's landing limit; an invisible pad marks it. The landing may be in plain view (OF D20). A touchdown more than 100 m off the LZ (or a timeout, or no pilot) is a failed insertion: the airframe flies home with its cargo, is removed unseen, and the operation goes back to abstract flight once (the airframe re-created at the next birth, no refund), then is cancelled unseen. No LZ, or no airframe: the operation goes on foot, its price refunded. A shot-down airframe is lost; whoever got out fights on. **Unload:** the cargo (a second group) gets out and becomes the operation's live squad, walking the straight line from the LZ; the airframe departs home onto the `AirHomes` list. **Removal:** an airframe is deleted only where `OccupierAir::UnseenAt` passes at its altitude for 2 s: past the band's bound in 3D from the camera (nothing is drawn there, so this holds even inside the view cone: a speck at altitude is unseen), or outside the view cone and terrain-hidden, never inside the air audible floor; `departTimeout` (900 s, past the band) is the backstop. The CREW is deleted FIRST (each man out of his seat first, row 23), the hull one step later, the pad with the hull. Then its price is refunded and the crew go home as bodies (`CAOpHullRefund`); a destroyed airframe is lost (`CAOpHullLost`). A strike (row 21) is live only while its target is in the bubble (a real sortie for `forcesSortieTime`, limited reveal of sightings under 60 s at accuracy 0.5, resistance AI only, never the player); otherwise its whole flight is abstract. Live airframes count against `forcesLiveHullsMax`; an insertion is two groups, a sortie one. Saved: the record's air fields, airframes flying home and OccupierAir's records (hull, crew, cargo and pad refs resolve on the second pass; a pad missing after a load is re-created). A base on another landmass than the target is never a ground source but stays an AIR-ONLY source (section 9.3). | `CounterAttack` (`CounterAttackAir.cpp`), `OccupierAir` |

**Known artifact of D1.** If the player leaves while his fighters are still
in a firefight at a post, the surviving occupiers are written back as the
reserve and the zone becomes a frozen standoff until the player returns and
the garrison rematerializes. Remote combat resolution is a #38 follow-up.
With `occupierForces` on, an operation never resolves a fight abstractly
(row 2c), so this artifact concerns only garrisons.

## 4. Configuration keys

All live in `class CfgGuerrillaZones` of a template's `description.ext`,
except `gmHeatDecayHeld` (a script global set in `core/init.sqs`).

**Zones, garrisons and the legacy counter-attack.**

| Key | Default | Meaning |
|---|---|---|
| `remoteTick` | 1 | D1 on (0 restores the old freeze outside `cacheRadius`) |
| `garrisonRecoverInterval` | 600 | D2 seconds per recovered body (0 = off) |
| `garrisonMax` (per zone) | the zone's `garrison` | D2 ceiling; for a CITY or a post also the revert refill |
| `cityGarrison` | 4 | occupier defenders of every administered town (whole number 0..24, repaired and logged otherwise); a CITY's own `garrison` overrides it; 0 = the pre-#79 free towns |
| `townRetake` | 1 | town retake on (0 = liberated towns are permanent and inert, the pre-#79 rule) |
| `townRevertSupport` | 30 | a held town reverts below this support, not below `supportFlip` (range 0..`supportFlip`, repaired and logged; `= supportFlip` is the pre-hysteresis rule) |
| `baseRetake` | follows `occupierForces` unless authored [code] | base recapture on (row 1b); 0 = held posts are permanent (the superseded decision R) |
| `counterAttack` | 1 | 0 = no legacy dispatch at all |
| `counterAttackFirstTownChance` | 0.4 | legacy dispatch probability for the first town (k = 0); range 0..1 |
| `counterAttackChance` | 1.0 | legacy dispatch probability for every later town (k >= 1); with the director AWAKE it no longer answers |
| `counterAttackSize` | 0.5 | the legacy force, as a share of `groupSize` (12): 6 bodies at war level 1 |
| `counterAttackWarLevelStep` | 0.25 | force x max(1, 1 + step x (war level - 1)), capped at one group of 12; range 0..2 |
| `counterAttackDelayMin` / `Max` | 120 / 300 | seconds between the flip and the launch (legacy, drawn per flip) |
| `counterAttackSourceKeep` | 0.25 | share of a base's `garrisonMax` that never leaves it (every record) |
| `counterAttackSpeed` / `counterAttackRoadFactor` | 4 / 1.3 | abstract speed (m/s) and road distance per straight metre, key off. With the key on travel uses the package's real speed (section 9.3) |
| `counterAttackBirthMinToTarget` | 250 | no force is ever born this close to its target (m, range 0..2000) |
| `counterAttackBirthGiveUp` | 600 | seconds a force waits when no birth point passes on its own route AND no other base's route passes (key on: then it tries the re-source once more and is cancelled unseen, every record, legacy DORMANT ones included; key off: it turns back and its strength returns to the base). A force held at the target edge by the player in the target counts toward it too, then is born at the edge unseen or cancelled unseen (range 30..86400) [commit 6e1ee27, 37e24ba] |
| `counterAttackDefenderWeight` | 3 | one fighter is worth this many attackers: the legacy odds roll, and the director's required lethality |
| `counterAttackRepelLoss` | 0.5 | share of a beaten legacy force that does not come home (key off) |
| `counterAttackLiveTimeout` | 1200 | seconds an unresolved live squad stays before it withdraws (only out of the player's sight); the road march gets its own allowance (road left at 1 m/s + 120 s) |
| `counterAttackMaxLive` | 2 | live infantry records at once (more wait) |
| `campaignEnd` | 1 | 0 = not a campaign: no win, no lose, and the template authors `occupierForces = 0` |
| `campaignEndDelay` | 10 | seconds from a win or lose latch to the end mode [16] |
| `simDiagInterval` | 60 | seconds between `[gmsim]` log lines (0 = off) |
| `budgetLiveBodies` | 130 | WARN above this many live AI bodies, all sides |
| `budgetSideGroups` | 40 | WARN above this many groups with a living member on one side (engine cap 63; groups with no living member are reported as `emptyGroups`, not counted) [round 3.5] |
| `emptyGroupRelease` | 60 [round 3.5] | seconds a group with no members and no links (no script handle, service record, waypoint or queued radio message) waits before the zone tick releases it (0 = off) |

**Occupier forces.** Defaults and ranges from [16], confirmed in
`OccupierForcesTuning` / `CounterAttackTuning` [code] unless marked.

| Key | Default | Meaning |
|---|---|---|
| `occupierForces` | 1 (engine default from phase 7, `OccupierForcesTuning::KeyDefault`) [phase 7, landing with this commit]; `campaignEnd = 0` missions author 0 | the layer: pool, director, real speeds, live resolution, waves, base recapture, the HQ meter, alarm answers, air |
| `forcesPoolMin` / `forcesPoolPerTown` | 24 / 2 | `poolStart = max(Min, R + PerTown x K)` (ranges 0..200 / 0..8) |
| `forcesPoolRefillHours` | 4 | hours from exhausted to full, garrisons included (0.5..48) |
| `forcesPoolFloor` | 0.2 | no operation in THIN below this share of poolStart |
| `forcesDirectorInterval` | 60 | director tick (s, 10..600) |
| `forcesTokensMax` / `forcesTokenInterval[]` | 2 / {600, 1050, 1800, 0} [2026-10-15; was {1800, 2700, 3600, 0}] | command tokens; seconds per token by band STRONG, STRETCHED, THIN, SPENT (0 = none) |
| `forcesOpsMax` | 2 | born operations in flight (0..6) outside STRONG; unborn ones do not count; one source, the `OccupierForces` tuning (`OccupierForces::OpsMax`; `CounterAttack::OpsCap` falls back to 2 while the service is inactive and adds one per living wave record) [commit 37e24ba] |
| `forcesOpsMaxStrong` | 3 [2026-10-15; 0 or 2 restores the old cap] | born operations in flight while STRONG: max(`forcesOpsMax`, this) (0..6) |
| `forcesLiveHullsMax` | 6 | live hulls at once (0..20) |
| `forcesWakeTowns` | 2 | distinct towns that wake the director (2..99) |
| `forcesHeatFull` | 40 | candidate heat (1..100) |
| `forcesUrgentHeat` | 50 | urgency entry heat (exit below 35, a constant) |
| `forcesTownCooldown` | 1800 | seconds a target rests after a repel or a cancel |
| `forcesRearmHeat` | 40 [round 3.5; 0 = no re-arm, the old rule] | heat a held zone is held at after its rest ends with fighters still reported, until an operation goes (0..100; OCCUPIER-FORCES.md 6.3) |
| `forcesPushoutQuiet` | 1800 | seconds without a sighting that make a held zone quiet |
| `forcesIntelDecay` | 1800 | seconds a sighting row lives |
| `forcesRetakeMin` | 4 | minimum retake bodies (1..12) |
| `forcesSpareCap` / `forcesSpareCapFloor` | 1 / 6 [2026-10-15; `forcesSpareCap = 0` restores the one-source rule] | a post retake or HQ assault no single base can source takes the largest single-base spare, never below the floor (OCCUPIER-FORCES.md 7.2) |
| `forcesSlowEta` / `forcesSlowCostShare` / `forcesAirEta` | 900 s / 3.0 / 1500 s [2026-10-15; 0 = off, the old choice] | the arrival rule of the package choice (OCCUPIER-FORCES.md 6.5); push-outs included since round 3.5 |
| `forcesHqAssaultMin` | 6 | minimum HQ assault bodies (1..12) [code] |
| `forcesLethalityMargin` / `forcesUrgentCostShare` | 1.2 / 1.5 | package choice (1..3) |
| `forcesControlMilWeight` | 0.6 | weight of the roster in control C |
| `forcesControlDwell` | 2 | consecutive due ticks before a wave fires (1..10) [code]; key absent from [16] |
| `forcesWaveThresholds[]` / `forcesWaveFinal` | {0.75, 0.50} / 1 | control thresholds (at most 3) and the final stand |
| `forcesWaveShare` / `Min` / `Max` | 0.5 / 12 / 48 | wave size, share of poolStart, clamped |
| `forcesWaveLead` / `LeadFinal` | 600 / 120 | seconds inbound |
| `forcesWaveRetakeShare` / `forcesWaveOps` / `forcesWaveCluster` | 0.5 / 4 / 2000 m | strikes: share of the wave, targets, cluster radius |
| `forcesWaveSurge` / `forcesWaveHub` / `forcesWaveWarLevel` | 12 / "" / 1 | final-stand surge, hub by name, occupier war level per landed wave |
| `forcesSortieTime` | 300 [16] | seconds a live strike spends over its target (range 60..1800; row 25); read from config since `bcca80e` [air] |
| `forcesOffMapEdge` | nearest | the parent nation's map edge for off-map air: "N", "S", "E" or "W"; absent or anything else = the edge nearest the target (row 25) |
| `forcesTruck`, `forcesArmedCar`, `forcesApc`, `forcesTank` | ladder classification | ground hull class overrides, read from the occupier FACTION descriptor, not `CfgGuerrillaZones`; the shipped descriptors author none [code]. FACTION-PACKS.md documents them |
| `forcesTransportHeli` / `forcesAttackHeli` / `forcesJet` (faction descriptor) | none (air is never classified from the ladder) | the air hulls; a package is offered only when its class is authored [commit bcca80e]. Authored for EAST (Mi17 / Mi24 / Su25), WEST (UH60 / Cobra / A10) and the @LoBo IDF, Egypt (both rosters), Syria and Jordan; none for GUER, Hizballah or PLO. The descriptor lint does not probe these classes yet (section 8) |
| `forcesLiveHullsMax` owner | | read by `CounterAttack` (6, 0..20), not by `OccupierForcesTuning` [code] |
| `standing` (per CITY) | 50 | initial occupier standing |
| `gmHeatDecayHeld` (script) | 0.25 | heat bled per 10 s by every GREEN resistance-held zone |

**Saves.** The D2 clock is saved per zone (`recoverClock` on the zone row); a
save from before it loads with a fresh clock. The S5 pending deposit is saved
on the zone row (`pending`, and `pendingOrigin` by zone name); a save without
them loads with nothing pending, and a fractional saved reserve loads floored
to whole bodies. The zone row also carries `garrisonExempt` (the new-game
START TOWN headquarters is exempted from its town garrison), `flipDay` /
`flipMinute` (the last flip to the resistance), `occCapture`, `standing`,
`lostCount` and `struckUntil` (saved since `6e1ee27`; a town strike writes
`struckUntil` since `bcca80e`). A save from before
issue #79 keeps its towns' saved reserve of 0: they stay free until D2
recovery regrows an administered town's reserve while the player is away.
`seenDay` / `seenMinute` mean "last time the player was within the bubble".
The occupier forces block, the counter-attack block, the HQ block and the
campaign end block are in section 10.5.

## 5. Island coverage targets

Observable checks, not marker counts. All apply inside the playable region:
`playRegion[]` when the template authors it (it then bounds the base seed,
the city seed and the START TOWN list); with no authored region the default
box (the authored zones' bounding box padded by 5 km) bounds the base seed
only and towns are seeded over the whole map (issue #76, ISLAND-PACKS.md
section 10).

| ID | Criterion |
|---|---|
| C1 | Every military hub the map models in the region is a zone of the matching type: airfield as AIRFIELD, harbour as SEAPORT, garrisoned compound as OUTPOST. An AIRFIELD or SEAPORT may be authored or seeded at runtime (`seedBases`, issue #76); an OUTPOST is always authored. A map with none says so in its template header. `guerrilla lint --bases` lists what the detector finds and what the template leaves out. |
| C2 | From the default Camp, a garrisoned occupier zone lies within 1.5 km and is reachable on foot in under 5 minutes. |
| C3 | At least one occupier zone pair within 3 km with a road path, so patrols are admissible from minute one (`gmTrafficDiag` at the Camp). |
| C4 | A progression ladder: at least 3 occupier military zones of rising garrison (about 6 / 8 / 12), the AIRFIELD (where one exists) the strongest. |
| C5 | Every CITY is within 3 km road distance of an occupier military zone at campaign start. |
| C6 | No zone centre in water or inside a walled compound; every centre reaches open terrain. |
| C7 | Seeded CITY zones are settlements inside the region (no labels, airbases, borders or islands). A Names label that names a detected airfield ("Egyptian Airbase") becomes that AIRFIELD's name and is dropped from the CITY seed by name (issue #76). |
| C8 | Every occupier base's road net joins the island network, so an operation sourced there can route to the towns it answers. The code tolerates a base whose roads are a pocket on the same landmass (the road-route join, section 9.3), but the cross-country leg is slower and less natural, so a template should still pass. A base on another landmass than the towns (an island) is never a ground source at all: it is an air-only source (section 9.3), so a template should not rely on it for ground answers. Abel: the Southeast Compound stands on its own island, about 800 m of deep sea from Le Port; the earlier join leg crossed the sea, and its W and S gates are closed wire gates [commit bcca80e]. The squad stall inside its west gate is resolved by the landmass rule: the Compound sources air only, and the offscreen fixture's SEBase moved to the Le Port quay. |

The occupation network per template, after the runtime base seed (issue
#76). "Authored" is what the template's `class Zones` carries; `seedBases`
is the template's resolved value (absent: 1 when it authors no AIRFIELD and
no SEAPORT, else 0); "Seeded" is what `gmSeededBases` reports on a fresh
start. Detection rules: ISLAND-PACKS.md section 10.

| Template | Authored hubs | `seedBases` | Seeded at runtime | Author's work left |
|---|---|---|---|---|
| Abel (Malden) | Airfield, Le Port Harbour (SEAPORT), Southeast Compound and Outpost (OUTPOST) | 0 (default: authors both) | none (the detector's airfield and harbour are covered by the authored ones: `gmDetectedBases`) | none for C5 since `generateBases = 1` (OCCUPIER-FORCES.md 5.7): a new game generates two OUTPOSTs (garrison 6 each; about [3975, 4775] south-west and [4175, 7025] west, names and spots seeded per campaign and stored in the save), which close the western gap (Goisse, La Pessagne, Vigny, La Riviere, Cancon, Chapoi, Sainte Marie were 2.6 to 4.7 km from any hub). C8: the Southeast Compound is an island, an air-only source (above). |
| Eden (Everon) | Outpost 1 to 3 (generated, garrison 6 each) | 1 (default: authors neither) | Everon Airfield [4841, 11323] (AIRFIELD, garrison 12: the ILS apron) and Chotain Harbour [7358, 6803] (SEAPORT, garrison 8: the concrete pier and quay block at [7392, 6835]); R 18 to 38, poolStart 74. `generateBases = 1` (OCCUPIER-FORCES.md 5.7) adds two OUTPOSTs (about [3525, 4925] and [5425, 9375]), outside R | a real garrison ladder in place of the three generated outposts (C4); Regina stays more than 2.5 km from any base (no site in reach passes the terrain rules) |
| Sinai | Wadi Checkpoint, Ras Nasrani Outpost (OUTPOST) | 1 (default: authors neither) | inside the default region {1827, -2192, 14100, 9600}: Egyptian Airbase [3038, 1595] (both hangar groups, one AIRFIELD; the Names label is no longer a CITY), Wadi Mandar Airfield [8068, 3725] (the Wadi strip), Egyptian Fire Base Airfield [7006, 9194], Aqaba Internation Airport [10894, 8176], El Tor Harbour [5997, 3233], Maqnah Harbour [10727, 3550] (the Sharm port), Israel-Jordan Border Harbour 2 [10696, 9548] (the Aqaba port). Outside it, not seeded: the Israeli Airbase (the world's ILS entry), Eilat's strip, the Suez and Taba harbours. R 86, poolStart 112. `generateBases = 0` until `playRegion[]` (#75) | the El Tor garrison, the ladder (C4) |
| Lebanon80 | Litani Checkpoint, Marjayoun Barracks (OUTPOST) | 0 (set by the template: no airfield or harbour inside the theatre by design, airpower is off-map, #35; Ramat David is the only ILS entry, 7 km south of it, and the Tyre quay wall is not a harbour) | none by design (`generateBases = 0` too) | the Tyre HQ compound, two border posts, the Mt Dov post |

Coverage verdicts per template, from the #76 verification runs:

| Template | C1 | C2 | C3 | C4 | C5 | C6 | C7 | C8 |
|---|---|---|---|---|---|---|---|---|
| Abel | pass (lint: the detected airfield, harbour and compound are covered by the authored zones) | unchanged by #76 | pass (above) | pass (Outpost 6, Compound 6, Harbour 8, Airfield 12; the generated OUTPOSTs 6) | pass on the straight line with the generated bases (every town within 2.5 km of a ground base on its landmass, `guerrilla_bases_generate_abel` g3); the 3 km road distance itself not measured | unchanged by #76 | unchanged by #76 | Compound air-only (above) |
| Eden | partial: airfield and harbour seeded; the detected walled army base [5122, 10475] and the Levie post [7524, 4182] are OUTPOST candidates the template does not author | not measured | not measured | pass (Outposts 6, Harbour 8, Airfield 12) | partial: with the generated bases every town but Regina is within 2.5 km straight line of a ground base (`guerrilla_bases_generate_eden` e3); road distance not measured | seeded centres on dry ground by rule (spawn square dry), not walked in game | unchanged by #76 | not measured |
| Sinai | partial: 4 airfields and 3 harbours seeded in the region; the El Tor garrison and other compounds are OUTPOST candidates only. Aqaba's airport and port (Jordan) fall inside the default region | not measured | not measured | pass (Outposts, Harbour 8, Airfield 12) | not measured | as Eden | pass for the airbase label (guerrilla_bases_detect_sinai: no CITY named Egyptian Airbase) | not measured |
| Lebanon80 | pass by design (seedBases = 0, the header says so) | unchanged by #76 | unchanged by #76 | unchanged by #76 | unchanged by #76 | unchanged by #76 | unchanged by #76 | unchanged by #76 |

"Not measured" means the #76 runs did not test that criterion; it is open,
not passed. "Unchanged by #76" means no zone was added there.

The outpost ladders and the western Abel hub are #38 / #76 authoring work
(see section 8); AIRFIELD and SEAPORT yields (plan 13) are not implemented,
so every base pays like an OUTPOST.

## 6. Budgets and instruments

| Budget | Threshold | Instrument |
|---|---|---|
| Live AI bodies, typical | <= 100 | `gmSimDiag` element 2, `[gmsim] total=` |
| Live AI bodies, hard cap | <= 130 (WARN above); a live birth over it waits, retried every 30 s [code], never dropped. An ambient traffic spawn that would breach it or the side's group budget (counting the most bodies its kind seats: civ 1, patrol 3, convoy 7) is skipped for that pass, counted (`gmTrafficDiag` slots 17 to 20, `[gmsim] skippedBudget=`) and WARNed at most once a minute; forced spawns are exempt, and `gmSimAdmit [side, bodies, groups]` gives scripts the same check, which the town population (`civilians.sqs`, round 3 A) and the shakedown pair (`shakedown.sqs`: 2 bodies in 1 occupier group, a refused launch waits for a later tick, counted in `gmShkSkipBudget`) apply before they spawn and the garrison cache does not apply yet [code] | same |
| Groups per side | <= 40 (WARN above; engine cap 63). A package counts its groups: foot 1, a mounted squad 2 (infantry and crew), a column 3 (plus the escort crew) (`ForcePackages::GroupsOf`, `GoLive` [code]); a helicopter insertion 2 (crew and cargo), a strike sortie 1 [commit bcca80e]; a birth over budget waits. The budget counts only groups with at least one living member (`CensusTallyGroup`): a group whose units were all `deleteVehicle`d, or whose members are all dead bodies, is outside it and reported as `[gmsim] emptyGroups=` (every side center) and `gmSimDiag` element 11 [code, round 3.5, video 4 finding: resistance groups 1 to 14 with 11 bodies]. Leftover groups (section 9.6) stay counted while a member lives. An empty group still holds one of the center's 63 ids, and group creation (`CreateSideGroup`, `createGroup`) refuses at 63 on its own, so a birth refused there waits. The engine has no empty-group sweep (only `deleteGroup`, `join` and the services' own `RemoveFromCenter` release a group), so the zone tick releases a group that has had no members at all for more than `emptyGroupRelease` seconds (60, 0 = off) and that nobody links: not a player's group, not remote or in a network session, and not held by a script handle, a service record, a waypoint or a queued radio message (`ForeignGroupLinks`), so a group created for a later spawn is never taken; each release is one INFO line and `[gmsim] released=` [code, round 3.5] | `gmSimDiag` element 3, `[gmsim] maxSide=`; `emptyGroups=`, `released=` |
| Live infantry records | `counterAttackMaxLive` (2, 0..8 [code]); strikes count as groups, not as live squads | `gmCounterAttacks` |
| Born operations | `forcesOpsMax` (2), `forcesOpsMaxStrong` (3) while STRONG [2026-10-15]; an unborn operation does not count; `CounterAttack::OpsCap` adds one per wave record (origin `CAOWave`) in any phase, and a wave order gets one more headroom at launch [code] | `gmOccupierOps` |
| Live hulls | `forcesLiveHullsMax` (6, 0..20, read by `CounterAttack`): a column counts 2, a live airframe 1; the AI census counts crews, not hulls | `gmOccupierOps` element 8 |
| Command tokens | 0..`forcesTokensMax` (2), full at the wake; one per 600 / 1050 / 1800 s in STRONG / STRETCHED / THIN, none in SPENT [16, 2026-10-15; was 1800 / 2700 / 3600]; one launch per director tick at most | `gmOccupierForces` element 3 |
| Guerrilla service block ("gm" frame phase) | avg <= 0.5 ms, p95 <= 1.5 ms; spawn-burst frames are logged, not gated | `triPerfStats` `gm=`, `[gmsim] gm=avg/p95/max` |
| Frame time at a hub scene, 1080p, default view distance | p95 <= 25 ms (pass); p95 > 33.3 ms is a fail | dev panel Profile tab, `[gmsim] frame=` |

Instruments:

- `FrameProfiler` phase `gm` times the Guerrilla service block in
  `World::Simulate` (zone, garrison, flags, traffic, assailants, stashes, HQ,
  market, legends, counter-attack, occupier forces). `triPerfStats` and the
  dev Profile tab list it.
- `gmZoneSim <i>` returns `[inBubble, guer, occ, seenDay, seenMinute,
  contested, lastRemote, garrisonMax, recoverClock, garrisonLive,
  garrisonExempt, flipDay, flipMinute, pending, occCapture, lostCount,
  standing, struckUntil]` (18 slots [code]) as of the last zone tick
  (`guer` is the CAPTURE presence; `occ` the positional count only;
  `pending` is the live S5 deposit). `gmZoneSet` writes `occCapture`,
  `standing` and `struckUntil` (tests). `gmZoneStruck <i>` is true while a CITY is inside
  its strike window (mission time < `struckUntil`) [commit bcca80e].
- `gmLandmass [pos]` returns the walkable landmass id of a point
  (`RoadRoute::Landmass`, -1 unknown or at sea); `gmGeoGrid [pos,
  halfCells, step?]` dumps the landscape's geography cells around it
  [commit bcca80e].
- `gmHqState` returns `[hasHq, destroyed, epoch, hqCapture, zoneIndex,
  pos]` (zone -1 and pos `[]` without an HQ); `gmHqEpoch` the epoch;
  `gmHqForceCapture <v>` sets the meter (0..100) and destroys a standing HQ
  at 100 [code].
- `gmCampaignEnd` returns `[won, endPending, endIssued, endClock,
  rosterHeld, rosterSize, wonDay, lastTaken, lost]`; `gmCampaignLost` the
  lose latch [code].
- `gmPlayerPresence` returns `[counts, undercover, exposed, weaponInHands,
  warriorOutfit, liberatedTowns, holds, weaponSeen]`, sampled now. `counts`
  is CAPTURE (issue #86): not undercover, exposed (a living occupier group
  holds a compromised record of him), or a weapon in hands; the outfit alone
  never counts. `holds` is HOLD (OF D11, OF D23): not undercover, or a
  uniform (weapon out or not); in a civilian outfit only exposed, armed, or
  `weaponSeen`. `weaponSeen` is true while a living occupier observer has
  rated him suspect or exposed because of a visible weapon within the last
  5 s (a slung rifle seen from behind or at conversational range counts;
  `gmUndercoverWeaponSeen` gives the group count). `warriorOutfit` is
  informational. CAPTURE raises a town's support and a post's capture meter;
  HOLD blocks the drain of a held town, the occupier's `occCapture` and the
  HQ meter.
- `gmCounterAttacks` returns one row per pending or running record, 29
  slots [code]: `[town, source, phase, clock, strength, size, townsBefore,
  group, spawnPos, spawnReason, opId, kind, origin, package, liveHull, eta,
  hullClass, funded, hull, seatedInfantry, leader, livingInfantry,
  escortClass, escort, airId, lz, airOrigin, dismounted, struck]`, phase
  SCHEDULED / TRAVEL / HOLDING / LIVE (slots 24 to 28 since `bcca80e`: the
  live airframe's `OccupierAir` id, the LZ `[easting, northing]` or `[]`,
  the air origin zone or -1 off-map, the infantry out of the airframe,
  `CAOpStruck` sent); `gmCounterAttackTune [key, value]` overrides one
  tuning value at run time (tests; a set "speed" wins over the real
  speeds). `gmCounterAttackLaunchFunded [target, source, package, bodies,
  urgent or kind, hullClass, kind, escortClass]` is a test lever (op ids
  from 900001; the units it spends get a lever ledger row, which settles
  refunds and losses through 5.5 and drives no director action, review L14); since `bcca80e`
  it also has air and alarm forms.
- `gmOccupierForces` returns 30 slots [code]: `[pool, poolStart, band,
  tokens, posture, control, wavesFired, K, excludedTowns, lost, townsEver,
  urgent, cooldowns, opCounter, tokenClock, waveInbound, waveClock,
  waveBodies, waveName, finalFired, surgeZone, surgeBodies, wavesLanded,
  occupierWarLevel, townsStruck, strikesApplied, alarmAnswers, legacySave,
  legacyQrf, probedOut, rearmed]`; band and posture "OFF" with the key off;
  probedOut the seeded towns the settlement probe kept out of K (10.1);
  rearmed (slot 30) the zone names re-armed after a rest (section 9.2)
  [code, round 3.5].
  `gmWarLevelOccupier` gives slot 23 alone; `gmLegacyQrf` slot 28 alone
  (true while `qrf.sqs` fields its script QRF, row 7).
- `gmOccupierOps` returns `[id, kind, origin, package, source, target,
  phase, bodies, hulls (0..2), eta s, birthReason, hullClass, escortClass]`
  per pending or active operation (13 slots [code]). Every director action
  writes one line starting `OccupierForces: [op N]`, N the same id
  (OF D12).
- The director log toggle [code, 2026-10-14]: the per-action lines of the
  director (`OccupierForces:`), `CounterAttack` / `CounterAttackAir`
  (`CounterAttack:`) and `OccupierAir` (`OccupierAir:`) log at INFO only
  under `--dev` or with `forcesLog = 1` in CfgGuerrillaZones (default 0,
  range 0..1, `gmForcesTune ["log", 1]` at run time), and at DEBUG
  otherwise (`--log-level debug` shows them). WARN and ERROR lines always
  log. Tests that read these lines from the game log pass `--dev` (OF 6.6).
- `gmOccupierIntel` returns the sighting table `[zone, count, age s,
  positions]`, so tests can pin OF D9.
- `gmForcesTune [key, value]` (every scalar `OccupierForcesTuning` key, not
  the arrays or the hub; some test ranges are wider than config: wakeTowns
  1, lethalityMargin 0, refillHours 0.01 h, directorInterval 1 s; "log"
  the forcesLog toggle) and
  `gmForcesForce ["pool" | "wave" | "refillHours" | "cooldown" | "strike"
  | "strikeEffects" | "alarm" | "legacySave", value]` are test levers
  [code] (the last four since `bcca80e`).
- `gmRoadRoute [from, to, joinRadius?]` returns `[found, points, length,
  startsSearched, startIndex, linksSearched, startOffset, startCandidates,
  goalCandidates, joined, joinGap, joinFrom, joinTo]`; `gmRoadComponents
  [pos, radius]` lists the road components near a point (size, links in
  radius, nearest, dangling ends, gap to a foreign end) [code].
- `gmOccupierAir` and the `gmAir*` levers (`gmAirInsert`, `gmAirSortie`,
  `gmAirExposure`, `gmAirUnseen`, `gmAirAbort`, `gmAirReveal`, `gmAirTune`)
  drive `OccupierAir` directly, as the spike did [air]; column layout in
  `OccupierAirCommands.cpp`. Director air goes through `CounterAttack`.
- `gmSimDiag` returns `[zonesInBubble, zonesRemoteTicked, [liveOcc, liveRes,
  liveCiv, liveTotal], [grpOcc, grpRes, grpCiv, grpMaxSide],
  garrisonSpawnedZones, garrisonLiveBodies, [civ, parked, patrol, convoy],
  [gmAvg, gmP95, gmMax], [frameAvg, frameP95, frameMax], breachBits,
  [farOcc, arrival, march, leftover, homeHull, air], [emptyGroups,
  released]]`. Element 3 counts groups with a living member; element 11
  the groups with none on every side center and the idle empty groups
  released since mission start [code, round 3.5].
  Timings cover the profiler's last 256 frames. Element 10 (and
  `[gmsim] far occ=`) counts `CounterAttack`'s live occupier groups beyond
  850 m of the player by cause (`CounterAttackFarCensus`) [code]; with the
  director log on, every `CAOpArrived` logs `arrived abstract|live`, and
  every `CAOpBorn` its `spawnReason`, each with the player distance.
- `gmSimDiagLog` writes one `[gmsim]` line now and returns the breach bits
  (1 = live bodies, 2 = groups on one side). The same line is written every
  `simDiagInterval` seconds.

Worst case at a hub, from the current defaults [contract]: garrisons in the
bubble 24, QRF 6, counter-attacks 12 (two squads of 6 at war level 1, up to
24 from war level 5; a squad marched out of a base in the bubble moves
bodies, it adds none), traffic crews about 10, civilians 24, shakedown 2,
assailants 8 (about 86 bubble-bound, 98 at war level 5), plus always-live
Legends (up to 18), the player group (up to 12), 3 per captured military
zone in hold squads, and dealers. Sinai's 26 dealers are the known outlier
(decision X). With `occupierForces` on, the counter-attack line becomes the
born operations: `forcesOpsMax` (2, 3 while STRONG) records of at most one infantry group of
12 [code] each, plus the crews of up to `forcesLiveHullsMax` (6) hulls
(a column brings 2 hulls and 3 groups, a helicopter insertion 1 airframe
and 2 groups), plus one more operation per wave
record; every one of them waits rather than
breach the 130 cap or the 40-group budget. A new measured worst case is a
phase 7 soak item (section 7).

## 7. Observable scenarios

Each scenario has an automated Trident test on the full Remaster lane
(`packages/RemasterUD`, tags `remaster_ud`, `offscreen`) using the fixture
mission `tests/integration/missions/guerrilla_offscreen.abel` (the native
parity mission plus a second occupier post, SEBase, D2 at 15 s and a 30 s
ambush window; `campaignEnd = 0`, `occupierForces = 0`). SEBase stood at
Malden's south-east compound until `bcca80e` moved it to the Le Port quay:
the compound is an island (C8).
The "far point" is Malden's airfield runway, 3.3 km from the Outpost.

| Scenario | Must persist | May change without the player | Test |
|---|---|---|---|
| (a) Leave friendly troops at a distant zone | The zone's intel stamp; a stored reserve; owner and capture during a standoff; support of a town where only occupiers stand | The capture meter climbs and the zone flips with the usual side effects; a town with an organizer gains support and rises; an unheld half-capture decays | `scripting/guerrilla_offscreen_capture` |
| (b) Wait in an untouched region | Every owner, support and capture; the war level; no garrison materializes; a full reserve never grows | A bled reserve regains 1 body per interval up to its authored strength; heat cools; income pays | `scripting/guerrilla_offscreen_idle` |
| (c) Interrupt a convoy, then leave and return | Reserves (the QRF is free, a convoy carries nothing); owners; no duplicate convoy on return | The attributed zone goes RED then back to GREEN; the QRF launches and stands down; heat rises then cools | `scripting/guerrilla_offscreen_convoy` |
| (d) Cross simulation boundaries and save/reload | A bled garrison stays bled across the bubble edge and across near and far saves reloaded in fresh processes; a distant standoff survives | Nothing, except D2 steps (kept out of the test's windows) | `scripting/guerrilla_offscreen_save_reload.seq` |

Counter-attacks on the shipped Abel town Houdan:
`scripting/guerrilla_counter_attack_live`, `_offscreen`, `_unseen_spawn`
and `_watched_base`. Abel now runs with the key on, so `_watched_base`
asserts the re-source of the birth chain (and the S5 pending deposit)
rather than the old turn-back [commit 6e1ee27].

Occupier forces, on the shipped `Guerrilla.Abel` (`occupierForces = 1`).
Every birth and pop-in test first asserts a real camera (`gmTrafficPercept`
`hasCamera`), pins daytime and the default view distance, and is not
headless.

| Test | Proves | Status |
|---|---|---|
| `guerrilla_forces_pool_off` / `_pool_on` | the layer inert with the key off (the 29-slot row since `bcca80e`); the pool sized and banded with it on | Landed |
| `guerrilla_forces_watched_route` | S: the whole Harbour route watched, the operation re-sourced to the next base back; the landmasses (the Southeast Compound on its own), no road route from the Compound, and the re-source to the Airfield | Landed (updated `37e24ba`, `bcca80e`) |
| `guerrilla_forces_no_popin`, `guerrilla_forces_wiped_release` | S5 | Landed |
| `guerrilla_forces_passerby_revert` | A parked patrol reverts an empty town (OF D5); the refill is pending and pool-paid | Landed |
| `guerrilla_forces_both_towns` | Two operations; INFO lines joined to `gmOccupierOps` ids | Landed |
| `guerrilla_forces_motorized` | Urgent far target: a truck squad drives and dismounts at 250 m | Landed |
| `guerrilla_forces_save_reload.seq` | Pool, bands, posture, tokens, operations and hull refs across a fresh process | Landed (phases 01_wake, 02_reload; waves not yet asserted in it) |
| `guerrilla_forces_base_retake` | STRONG pool: the director retakes a post; a hold squad forces a live fight | Landed (`6e1ee27`) |
| `guerrilla_forces_hq_assault` | Default camp start has a real HQ; a patrol through the Camp does nothing to it; an assault fills the HQ meter; destroyed with cache and garage; re-established in an enemy town whose garrison still defends it (OF D24) | Landed (`c825a5f`) |
| `ui/remaster/remaster_ud_guerrilla_forces_lose` | No HQ, base or town: the defeat debriefing, `triDisplay == 50`, the lose text on control 102 | Landed (`c825a5f`; doc 12.2 calls it `guerrilla_forces_lose`) |
| `guerrilla_forces_refill_pushout` | `gmForcesForce ["refillHours", 0.05]`: refill, then push-out | Landed (`6e1ee27`) |
| `guerrilla_forces_wave_base` | Wave 1 strikes the densest cluster and retakes a base; no win while it is enemy-held | Landed (`6e1ee27`) |
| `guerrilla_forces_armoured_column` | A carrier and its escort born unseen under the headlight gate, kept together (the 150 m / 60 m rule), the escort supporting at the edge | Landed (`37e24ba`) |
| `guerrilla_forces_heli_insertion`, `guerrilla_forces_jet_sortie` | The air spike's recipe on explicit birth points (section 12). Tag `slow_air`: default runs pass `--skip-tag slow_air`; this lane runs `--tag slow_air --retries 3` | Landed as spike tests (`c5cfa44`), not director behaviour |
| `guerrilla_forces_air_op` | A RETAKE bought as a helicopter insertion from the Airfield: the Mi17 born airborne beyond perception, the LZ 300..500 m out, a landing within 100 m of it, the cargo the live squad, the airframe removed unseen crew first. Tag `slow_air` | Landed (`bcca80e`) |
| `guerrilla_forces_air_save_reload.seq` | An insertion saved mid-flight (`01_inflight`) resumes in a fresh process (`02_resume`) with its air fields and OccupierAir refs. Tag `slow_air` | Landed (`bcca80e`) |
| `guerrilla_forces_town_strike` | A forced strike: -40 / -40 in the struck town, -10 / -10 within 2 km, nothing beyond; `struckUntil`, `gmZoneStruck`, the `townStruck` panic, OF D18 (no passive recovery), half Roubles while struck. Tag `slow_air` | Landed (`bcca80e`) |
| `guerrilla_forces_alarm_answer` | A RED Outpost answered by one ALARM operation from another base while DORMANT, born and marching on it, with `qrf.sqs` silent; `legacySave` brings the script QRF back (row 7) | Landed (`bcca80e`) |
| `guerrilla_forces_rearm_town` | A held Houdan with three fighters a local reports rests 120 s; at the rest's end it is re-armed (`gmOccupierForces` [30]), its heat held at 40 against the decay, and a DIRECTOR retake launches within one token interval (row 9, section 9.2) | Round 3.5 |
| `guerrilla_forces_strike_band.seq` | `01_refused`: the ground operation leaves the pool STRONG but no airframe would, so no strike goes; `02_bought`: a full pool, the opener goes and the band, sampled every frame, stays STRONG (row 21) | Round 3.5 |

Budget soaks tour whole islands through the GUERRILLA menu with the shipped
templates: `ui/remaster/remaster_ud_offscreen_budget_abel` and
`ui/remaster/remaster_ud_offscreen_budget_sinai_lobo`. Results are in
[FULL-SCALE-SESSION.md](FULL-SCALE-SESSION.md). Phase 7 reruns both with the
layer on by default and records the new worst case in section 6.

## 8. Follow-ups

Owned elsewhere; this contract only fixes the rules they must follow.

- **#38 full mode:** author the section 5 hubs per template (C8 included);
  fix Sinai seeding (C7) and cap dealers; scaffold detection of airfields,
  harbours and compounds plus a `playRegion[]` key; expansion under
  passivity on the abstract tick (row 15); caching friendly hold squads as a
  resistance reserve that still counts for D1; remote combat resolution for
  frozen garrison standoffs. Military recapture is no longer a #38 item
  (decision R).
- **#35 occupation director:** town security (#72) adopts the
  materialization rule; checkpoints as persistent records on road edges;
  convoys as carriers of D2 reinforcements; remote closures and reprisals on
  the abstract tick, deterministic and journaled. The QRF drawn from a pool
  is the ALARM operation of row 7 (landed in `bcca80e`).
- **#72:** the partial-spawn leak is closed with the key on (the shortfall
  goes back to the pool, section 10.2); with the key off it is still logged
  at WARN, not refunded. Town defender placement is the hashed
  `GarrisonCache::SpawnPoint` set in the square around the marker (L11),
  and a YELLOW alert still sends every cached garrison group toward the
  last-known position, except in THIN and SPENT, where
  `qrf.sqs` skips the YELLOW chase (reads `gmOccupierForces` slot 2)
  [code].
- **Review items closed after `08c6ae2`** (`tmp/of-review-phases-1-3.md`):
  L6 (the ledger keeps a row with units outstanding), L8 (a phase-2 block
  seeds `_townsEver` from the owners), L10 (row 4: a reserve write before
  the first tick after a load), L11 (row 4: the release checks the bodies'
  own spawn points), L13 (a live force whose road ahead passes the player
  stays live; a hull whose base fell drives to the nearest occupier base or
  stops), L14 (lever ledger rows; NaN tuning values take the low bound),
  L15 (a recorded bisect gap, OCCUPIER-FORCES.md 18), and the live top-up
  room cap (rows 7 and 10.2). Declined: `ArriveLive` with the source inside
  `counterAttackBirthMinToTarget` + 1 m of its target (not reproduced: it
  needs a source base within 251 m of its target; the give-up path then
  cancels unseen and the holding path waits at the edge by design, 6.5);
  `gmForcesTune "enabled"` not persisted (a test lever; the key is the
  mission config).
- **Phase 6 leftovers** [commit bcca80e]: a slow native alarm answer could
  contest a later Showcase capture chapter. Closed in phase 7A: the
  descriptor lint probes the `forces*` classes (a missing one is WARNed and
  cleared). Closed after `509e30d` (#97): the air re-plan's helicopter price
  now goes on the operation's ledger row (`OccupierForces::SpendOnOp`,
  section 9.3 step 7) instead of bypassing it. Accepted deviations, not defects: no attack-helicopter escort
  for an insertion; strikes are live only in the bubble; wave strikes buy
  no air opener; an HQ strike with no report aims at `HqPos`; a
  base-opener strike cuts only the 2 km ring.
- **Closed since the draft:** `Traffic.cpp` [commit 2524c3a] and
  `OccupierAir.cpp` [commit bcca80e] move a seated man out before deleting
  him (row 23); the return-to-abstract and top-up removals wait for the
  dead too (`BodiesUnseen` [commit bcca80e]), so a corpse in view no longer
  vanishes; the Southeast Compound's gate stall is resolved by the landmass
  rule (C8).
- **Separate issues:** respawn at the HQ; player-facing messaging for the
  director, waves, strikes and the HQ (OF D12: INFO lines and gm* commands
  only).

## 9. Pacing and movement

### 9.1 The first town, and the legacy dispatch

The rule (issue #79, set by the user on 2026-10-12): the first town the
resistance takes may or may not draw an immediate answer. The resistance
gets no bonus for holding towns: every town rises at the plain
`supportRate`.

This dispatch is the whole answer with `occupierForces = 0`. With the key on
it answers only while the director is DORMANT: the first distinct town, and
a re-flip of that same town (invariant W, OF D7). From the flip that brings
the distinct set to `forcesWakeTowns` (2) on, the director takes every flip
(section 9.2).

**Counting towns.** k is the number of towns the resistance had already
taken before this flip: the larger of a saved counter that every flip of a
non-exempt CITY to the resistance raises by one, and the non-exempt towns
the resistance holds at the flip. The town the HQ stands in (`HqZone`) never
counts and is never a target (N4a).

**Dispatch.** k = 0 is answered with probability
`counterAttackFirstTownChance` (0.4); k >= 1 with `counterAttackChance`
(1.0). The roll is a hash of the flip (town, flip stamp, k). The force is
`min(12, max(1, round(counterAttackSize * groupSize * max(1, 1 + counterAttackWarLevelStep * (warLevel - 1)))))`
bodies: 6 at war level 1, 8 at 2, 9 at 3, 11 at 4 and 12 from 5 [contract],
capped by the nearest base's spare,
`(stored reserve + live garrison) - floor(garrisonMax * counterAttackSourceKeep)`.
It leaves after a delay drawn from [120, 300] s.

**The odds (key off only).** A force arriving off-screen at a town with
fighters rolls once: the town holds with `p = D*w / (A + D*w)`. Three
fighters against six attackers hold with `p = 9 / 15 = 0.6` [contract]. A
beaten force loses `ceil(A * counterAttackRepelLoss)`. With the key on there
is no roll: the arrival goes live (row 2c), and a legacy record travels at
foot speed (1.5 m/s [code]) over the real road length.

**What the player should expect, key off** [contract]:

| | Town 1 (k = 0) | Town 2 (k = 1) | Town 3 (k = 2) |
|---|---|---|---|
| A force comes | 4 times in 10 | always | always |
| When | 2 to 5 min after the flip | 2 to 5 min after the flip | 2 to 5 min after the flip |
| Force planned, war level 1 / 3 / 5+ | 6 / 9 / 12 | 6 / 9 / 12 | 6 / 9 / 12 |
| Three fighters stay, war level 1: town lost | 0.16 | 0.40 | 0.40 |
| Three fighters stay, war level 3: town lost | 0.20 | 0.50 | 0.50 |
| Left empty: town lost | 0.40 | 1.00 | 1.00 |

With the key on only the Town 1 column applies, minus the odds rows.

### 9.2 The director (key on)

**Posture.** DORMANT until the distinct set reaches `forcesWakeTowns` (2);
then AWAKE, latched, tokens full. Before then the director considers nothing
(invariant W).

**Intel (OF D9).** Every 5 s the scan records the resistance units each
living occupier group holds as known targets, by reported position. A zone's
row is replaced by its latest scan and kept, a destroyed group's last report
included, until `forcesIntelDecay` (1800 s). `knownFighters(zone)` is the
row's distinct count. The director never reads the true census, the
player's position or `AlertMachine`'s position fallback, with one
deliberate exception: an informant tip. Each director tick a CITY with
standing >= 50 and resistance HOLD presence rolls (hashed, salt 11)
against `standing / 200`; a tip writes the town's TRUE HOLD count at the
town centre, jittered up to 100 m per axis, into the sighting table, and a
fresher soldiers' report wins over it [code]. Accepted: 7.3 frames tips as
D9-legal ("a local talked"), so doc 6.1's "never the true census" must name
this exception.

**Candidates.**
- *Towns.* A held town (not the HQ's, not on cooldown, no record on it) is a
  candidate while `heat / forcesHeatFull >= u`, u a roll fixed per flip
  (salt 7 [code]). A fresh quiet flip (+40) is a sure candidate, then fades
  with `gmHeatDecayHeld`.
- *Posts* (OF D16). The same rule, only while STRONG, never the HQ's zone;
  minimum bodies `min(max(forcesRetakeMin, garrisonMax), 12)` [code],
  lowered by the spare cap when no single base can spare it: the largest
  single-base spare, never below `forcesSpareCapFloor` (6) [2026-10-15].
  Below the floor the operation is refused and the line names the
  shortfall ("12 needed, the largest single-base spare is 6: short 6").
- *The HQ* (OF D13, OF D14, OF D24). Only with an established HQ, no HQ
  record and no "@HQ" cooldown. While STRONG or in a wave (inbound, or
  landed within 1800 s) by the heat of the zone it stands in; an HQ outside
  every zone is never a heat candidate. Unconditionally, first, the final
  push: when the resistance owns no post and no town OTHER than the HQ's
  own zone (`FinalPush`; the exemption flag is not checked) [code]. Always
  an HQ_ASSAULT on `HqPos`, minimum `forcesHqAssaultMin` (6) bodies. The
  code wins: doc 6.3 says "no base or town" without the HQ-zone exception.

**Urgency.** Latched by a YELLOW or RED edge on the target since its flip,
or heat >= 50; released when heat < 35 [16]. A fresh quiet flip is never
urgent.

**Tokens and launches.** Each director tick with a token, a candidate,
`CanOperate()` and fewer than `forcesOpsMax` born operations (`forcesOpsMaxStrong` while STRONG) launches ONE
operation on the first candidate (the next one when the first cannot be
served); the order is urgent first, then whole heat points, then value (a
base 3, a town within 1 km of an occupier base 2, a far town 1, the HQ 2),
then heat [code, 2026-10-16, video 5's two towns re-armed at 40]. The director quotes each source's STORED spare (minus a wave
surge), the same figure `CounterAttack::LaunchCore` grants from; a refused
source is zeroed and the launch re-planned on up to 4 other sources
(`MaxSourceRetries`), stopping if the pool refused (review M1) [code]. A
repelled director operation (never a legacy op 0, review M6) puts its
target on `forcesTownCooldown` (1800 s, +- 20 % hashed on the zone and the
operation id, `JitteredRest` [code, 2026-10-16]) with +15 heat unless a RED edge
paid it or it was a REINFORCE; a cancelled-unseen one gets the cooldown
only [code]. When a rest ends on a zone the resistance still holds and the
director's sighting table still reports fighters there (its men or an
informant tip, row 22), the zone is re-armed: its heat is held at
`forcesRearmHeat` (40, the candidate heat, so every roll lists it) on every
director tick until an operation is launched on it, its reports decay or
it changes hands; saved [code, round 3.5]. Before it, a defended held town
whose +15 decayed inside the rest and whose tips kept it from being quiet
fell out of reach (video 4: Arudy, 21 minutes of a STRONG director with two
tokens and nothing to do).

**The repel ladder and massing (OF 6.3) [code, 2026-10-16, video 5].** Per
zone the director remembers the repels of its own town and base retakes
(any origin, never the HQ), the last package and its source; saved, and
reset when the zone changes hands. One repel with fighters reported: the
next plan needs `L_req x 1.5` and at least twice the fighters as infantry.
Two: a different approach (another base that can staff it, else an
insertion after a ground answer, a ground answer after an insertion), and
an escalation strike may go if the pool stays STRETCHED (OF 7.3). Three:
contained, saved: no candidate, no push-out, no re-arm; the nearest occupier
base on its landmass is surged by the bodies the next operation would have
needed (a pool-funded deposit, pending inside the deposit bubble like any
reserve write); lifted when its reports decay, when a wave fires or when
control crosses a wave threshold. A director candidate whose escalated plan
is under strength holds its token ("massing for X") until a base can staff
it, a further token arrives or `forcesMassingMax` (3) ticks pass, then the
strongest one-base package goes; never on a first attempt; not saved. The
ladder reads only the director's own operation events and its sighting
table (D9). One INFO line per rung.

**Push-out (OF D8).** STRONG, a token and no heat candidate: the held town
or post nearest an occupier base with no sighting for
`forcesPushoutQuiet` (1800 s), with the cheapest package under the arrival
rule (below) since round 3.5 (never a contained zone): a foot quote under
`forcesSlowEta` still walks, a longer one may drive or fly in (until then a push-out kept the
minimal package whatever the walk; video 4: 55 minutes from the Airfield to
the Outpost) [code]. D2 regrows the posts meanwhile, so a quiet player
watches the occupier creep back.

**Packages (OF D1, OF D2).** Prices in pool units [16]: body 1, truck 3,
armed car 4, APC 8, tank 14; transport helicopter 10, attack helicopter 18,
jet 22, x 1.5 off-map. Lethality [16]: a body is worth its tier factor (1.0 /
1.25 / 1.5); carried infantry x 1.25; armed car + 3, APC + 6, tank + 12,
attack helicopter + 14, jet + 16. Required lethality `L_req = 1.2 x 3 x
knownFighters`; minimum bodies `max(forcesRetakeMin, garrisonMax(target))`
for retakes, 4 for alarm answers, 6 for HQ assaults. The cheapest package
meeting both goes; an URGENT target takes the fastest costing at most 1.5 x
that. The arrival rule [2026-10-15]: when the cheapest lethal package needs
more than `forcesSlowEta` (900 s, straight distance over its speed), any
faster package within `forcesSlowCostShare` (3.0) x its price may go, and
past `forcesAirEta` (1500 s) a helicopter insertion may go whatever its
price; the fastest eligible goes. It applies to the director's own
operations, push-outs included since round 3.5, and to alarm answers
outside THIN and, since 2026-10-16, to wave strikes from the hub (video 5's
HQ assault walked 41 minutes) [code]. No war-level gate. The source base pays the infantry from its stored
spare; the pool pays hulls and crews. Offered: every ground package, foot,
truck, armed car, APC and tank, and columns (`DefaultPackages` [code]);
and since `bcca80e` the air packages, each only when the occupier faction
authors its class (`forcesTransportHeli`, `forcesAttackHeli`, `forcesJet`;
section 4). The transport helicopter joins every package choice, from the
occupier-held AIRFIELD nearest the target, else off-map at x 1.5 (`airOrigin`
-1, `PackageRequest::airOffMap`); the strike airframes carry nobody
(`SeatCap` 0), so a ground or alarm operation never chooses one, and they
are bought only as strikes (row 21). A source base on another landmass than
the target is quoted for the helicopter insertion only
(`PackageSource::airOnly`, section 9.3). An insertion carries one airframe:
no attack-helicopter escort (an accepted deviation). A hull carries at most
`min(12, transportSoldier)` infantry; a hull that cannot seat the minimum is
quoted `seatsShort` and never chosen, so a tank or a two-seat armed car
goes only as an escort.
**Foot downgrade** (`MustDowngrade`, applied in `LaunchCore`): when a hull
package's stored spare cannot pay the bodies but the source can on foot
(its live march), the package goes on foot, same source and bodies, and
the whole price, escort included, is refunded (`HullRefund` "no stored
spare: on foot"). A hull that fails to spawn at birth also sends the force
on foot with the carrier price refunded [code].

**Won or lost.** Once `CampaignEnd` latches the win or the loss, nothing
launches, nothing in flight steps, nothing reverts [code].

### 9.3 Unseen births (decision S, the hard rule)

A soldier, hull or crew of any occupier operation is never created where the
player can perceive it. The user's rule of 2026-10-13 and OF D6.

**The landmass rule** [commit bcca80e]. `RoadRoute::Landmass` labels the
landscape's 50 m geography cells joined to each other over land (8
neighbours; deep water parts them, a road or track across water is a
bridge), once per landscape; a point on deep water takes the nearest land
cell within 2 cells (a quay or a pier); -1 means unknown, and
`SameLandmass` is false only when both ends are known and differ. A ground
force never comes from a base on another landmass than its target: the
legacy source pick (`PickSource`), the re-source (`Resource`, with an INFO
line naming the base passed over), the director's sources
(`BuildSources`) and the wave fallback all skip it; and a Traffic vehicle
never drives to a zone on another landmass than its origin (`PickDest`).
Such a base stays an AIR-ONLY source: the director quotes only a
helicopter insertion from it (`PackageSource::airOnly`), and the chain
below re-plans a ground operation as air at once when no other base shares
the target's landmass (step 7). Malden's Southeast Compound stands on its
own island, about 800 m of deep sea from Le Port, so on Abel it is an
air-only source. `gmLandmass` and `gmGeoGrid` expose the labels (section
6).

The chain:

1. **The gate.** Traffic's issue #53 gate (`PerceptionCamera` +
   `CanExposeSpawn` against `EffectiveBand`) rates every candidate point.
   Checked in order (`EngineOps::Exposure` [code]): sea (surface below
   0.5 m) is seen; inside the audible floor `trafficMinSpawnDist` (300 m
   [16]) is seen, even when hidden; at or beyond the effective band
   (`EffectiveBand` min-spawn distance from the camera) is hidden; a
   blocked terrain ray is hidden; inside the 120 degree view cone is seen;
   otherwise cone-only. A cone-only point counts only within 300 m of
   route arc from the source base (`ConeOnlyBaseRadius`) and only when no
   hidden point exists; no point within `counterAttackBirthMinToTarget`
   (250 m) of the target, ever. **The hull gate** [code]: every birth and
   deletion of a ground hull uses the hull band, which at night
   (`NightEffect` above 0.2) is Traffic's headlight band, with a 2.5 m
   (roof-high) ray in place of the infantry 1.8 m (review M8).
2. **At launch.** The road-snapped source base, then road route points
   toward the target every 50 m (`RouteStep` [code]), up to the 250 m bound.
   A base's live garrison inside the bubble marches out instead (it already
   exists). A road longer than 2 x the straight distance + 300 m is replaced
   by the straight line.
3. **The road route** (`RoadRoute`, OF 3.1 [code]). Stage 1 is a multi-start
   road A* (starts within 300 m, widening to 900 m; a start whose component
   is already exhausted costs nothing). Stage 2, the join, runs when the
   source's roads never reach the target's net: it follows the source's
   pocket road and leaves it at the link minimising road so far + 2 x the
   cross-country gap + the straight distance on to the target, crossing to
   a town-net link within 2000 m of the source
   (`RoadRouteServiceJoinRadius`), the best by that cost, not the nearest.
   The join lands only on the source's landmass [commit bcca80e]: it never
   crosses the sea. (The Southeast Compound on Abel, whose roads are
   pockets 1292 m from the island net, was routed this way until
   `bcca80e`; the leg crossed about 800 m of sea, and the Compound is now
   an air-only source under the landmass rule above.)
4. **On the road.** A force the player walked up to searches BACKWARD from
   its progress point toward its base, never ahead of where it has got to.
5. **Next base back, at once.** When no point passes on its route, the
   operation is re-sourced immediately (`CounterAttack::Resource` [code]):
   to the other occupier military zone not yet tried that stands on the
   target's landmass and whose STORED spare covers the whole force, nearest
   the target by straight distance first, whose route yields a point
   passing the gate (the probe runs under the hull gate; a base route is
   the road A*, else the straight cross-country line, an accepted
   deviation that the landmass rule keeps on land). The old base is refunded room-capped, the
   surplus or a lost base's share to the pool; the new base pays; the
   arrival clock restarts from the new base. Then each further base.
6. **Wait.** When no base passes: the force waits with its arrival clock
   stopped, doing nothing to the target, and every 30 s retries its own
   route and the re-source. After `counterAttackBirthGiveUp` (600 s) it
   tries the re-source once more.
7. **Air re-plan** [commit bcca80e] (`CounterAttack::AirReplan`; OF 3.1
   step 4). After the wait's last re-source fails, a FUNDED ground
   operation becomes a helicopter insertion when the catalogue offers the
   transport helicopter, its seats hold the bodies and the pool pays it
   (`OccupierForces::Spend`); it does so AT ONCE, without the wait, when
   no other untried base shares the target's landmass (waiting cannot help
   a ground force). The airframe flies from the occupier-held AIRFIELD
   nearest the target, else off-map at x 1.5; the ground hull's price goes
   back through the ledger, and the airframe's price goes on the
   operation's ledger row like a ground hull (`OccupierForces::SpendOnOp`:
   the row's package becomes the transport helicopter, its outstanding
   grows by the price), so the airframe's refund or loss settles through
   the row. Only a re-plan no row can take (no row for the operation, or a
   save from before this change) is settled straight to the pool
   (`airReplanned`). The insertion then follows row 25.
   A legacy (unfunded) record never re-plans.
8. **Cancel unseen.** Otherwise the operation is cancelled while still
   abstract: bodies back to their current source room-capped (the rest to
   the pool), the hull and escort prices refunded, the target on the
   1800 s cooldown, one INFO line. An unborn force vanishing off-map is not
   a visible turn-back.
9. **Edge hold.** A force whose progress point is already inside the bound
   while the player stands in (or the bubble covers) the target is held at
   the target edge, doing nothing. The hold counts toward
   `counterAttackBirthGiveUp`; then it is born at the edge out of sight
   (`ArriveLive`, searching back from the last route point beyond the
   bound), or, with no such point, cancelled unseen. A suspended HOLDING
   force does the same (review M2) [code]. A born-at-edge attempt that must
   wait for a budget drops back into the hold for another full give-up.
10. **Every record, key on.** With `occupierForces` on, legacy DORMANT
    records take the same chain and never turn back (OF D6). With the key
    off an unfunded record waits and turns back after
    `counterAttackBirthGiveUp`, its strength returning to the base, and an
    edge hold retries every 30 s with no give-up [code].
11. **Slots.** An unborn operation does not count against `forcesOpsMax`.

**Movement.** No fixed timers with the key on: foot 1.5 m/s, ground vehicles
0.6 x the slowest hull's `maxSpeed`, aircraft 0.8 x the airframe's
`maxSpeed` (km/h converted) [16, code]; a column moves at the slower
hull's pace. Born squads march SAFE by road and take their combat posture
at the target edge (foot: `zoneArea` + 100 m). Vehicles are driven by
`IssueCommand` to the driver; the mounted edge is 250 m, where the carrier
brakes and the infantry dismounts once it is at or below 1.5 m/s or after
15 s (so the dismount can happen inside the bound). A marching force whose
leader and target are both beyond the deposit bubble, whose squad is
unseen and whose hulls pass `HullUnseen` for 10 s (`VanishHold` [code])
returns to abstract travel at its progress, survivors and clock carried
over: the cargo group first, then each hull's crew, then the hull; no
refund (dead crew come off the price), re-created at the next birth.

**Live forces and the bubble.** A force at its target stays live until it
resolves, whichever way the player moves. It resolves as: wiped out
(repelled); the target reverts under it (it becomes the garrison, its
bodies coming off the revert refill, U2); or `counterAttackLiveTimeout`
passes out of the player's sight (it withdraws, survivors home). A live
birth over the 130 cap, the 40-group budget, `counterAttackMaxLive`,
`forcesOpsMax` or `forcesLiveHullsMax` waits and retries every 30 s, logged
once, never dropped. A force that comes home to a base inside the deposit
bubble is a pending deposit (S5).

**A travelling record whose target reverts under it** (a passer-by did it)
continues as a REINFORCE column when the town is not exempt and its
reserve, pending deposit and live garrison are short of `garrisonMax`,
else goes home (key on); key off it turns back [code]. An abstract
REINFORCE arriving puts in at most the room (`garrisonMax` - live -
reserve - pending, pending inside the deposit bubble) and sends the rest
home through the room-capped return, surplus to the pool (review L2).

**Top-up withdrawal** (review M5) [code]. A live REINFORCE squad whose
target is no longer the occupier's walks (or drives) home, re-ordered every
30 s, and is removed only once its leader is beyond 850 m and the squad is
unseen; survivors go to the source room-capped. If the town turns occupier
again first, it resumes.

**Tests.** `guerrilla_counter_attack_unseen_spawn`,
`guerrilla_counter_attack_watched_base`, `guerrilla_forces_watched_route`,
`guerrilla_forces_motorized`, `guerrilla_forces_armoured_column`; unit
`[counterattack]` birth-chain, column, leftover, edge-hold and top-up cases;
`guerrilla_forces_heli_insertion` (the gate at altitude, spike);
`guerrilla_forces_air_op` (an insertion through `CounterAttack`);
`guerrilla_forces_watched_route` (the landmass rule and the re-source to
the Airfield); unit `[counterattack]` air re-plan and landmass cases.

### 9.4 Accidental retakes (OF D5, D18, D19)

Any living occupier unit inside a held town's `zoneArea`, with no resistance
HOLD presence there, drains it; inside a held post it fills `occCapture`.
There is no ownership filter: traffic patrols, a column bound for
another target, Legend guards parked in a held town and the QRF all count,
on purpose. A patrol drains only while it is inside the 150 m circle, so a
passing column costs a few points of support, not the town, and the town
stays weakened until the player or his fighters return (OF D18). A patrol
that kills the 3-man hold squad and stays takes the post. None of them ever
touches the HQ (OF D14). Shakedown pairs no longer spawn in held towns (the
organic picker checks the owner; the forced debug hook does not).

### 9.5 Saves and seams

Pending and running records are saved (`GuerrillaCounterAttack`: target and
source by name, kind, origin, package, op id, price, phase, clocks, the roll
seed, a live squad's group, hull refs resolved in a second pass, and the
towns-taken counter). An old save loads its records as RETAKE / legacy /
foot, op id 0. Every roll is a hash of a seed fixed at the event, so a save
and reload never changes an outcome; package choice is reproducible to
within one heat-decay step.

`CounterAttack::Decide` is the legacy dispatch decision; `LaunchFrom` sends
a legacy force of a chosen size from a chosen base; `LaunchFunded` starts a
director operation from a fixed source. `CounterAttackOrders.hpp` is the
contract between the director and the movement service: one terminal event
per record (Repelled, Settled, CancelledUnseen or Aborted), Returned before
it, HullRefund or HullLost after it (each hull of a column refunds or is
lost apart; a cancel refunds `OwedUnits`).

### 9.6 Columns and the hull lifecycle (phase 5, `863b8aa`, `37e24ba`)

**A column** is a CARRIER (truck or APC, which carries the bodies) plus at
most ONE ESCORT (armed car, APC or tank, which carries nobody); foot never
takes an escort. It counts 3 groups (infantry, carrier crew, escort crew)
and 2 hulls [code]. Price: bodies + both hulls + both crews (crews paid as
bodies); lethality: the carrier's (carried infantry x 1.25, an APC +6) plus
the escort's bonus; pace: the slower hull [16]. Crews are seated driver,
then gunner, then commander where the hull has the seat; crew with no seat
are not spawned and their price goes back to the pool ("partial spawn").

**Birth.** The escort is born `EscortGap` (20 m [code]) behind the carrier's
birth point along the heading, at a spot that is hidden or no more exposed
than the carrier's, under the same hull gate (section 9.3).

**Keeping together.** Both hulls drive the same legs. The carrier waits
while the escort is more than `EscortSpread` (150 m [code]) behind and
drives on once it is inside `EscortRejoin` (60 m [code]); without this a
T72 falls hundreds of metres behind a Ural over 3 km [commit 37e24ba].

**At the edge.** From the carrier's first braking frame the escort drives
up to the town edge (`zoneArea` out from the target) and supports, crew
AWARE and firing; an armed carrier supports too; a truck holds back where
it stopped (`ForcePackages::EdgeRole`) [code].

**Removal.** A man sitting in a vehicle is moved out of it first, then
deleted (row 23), crew before hull. A hull is deleted only where
`HullUnseen` holds: the hull point under the hull band, and every crew
member not seated in it (dead ones included) under the infantry band
(review M4) [code]. Home unseen, each hull refunds its own price; on a
return to abstract travel both are deleted with no refund.

**The Leftover list** (review M3, L7) [code]. A lost hull's crew group, an
intact hull whose crew is dead or gone, and every wiped or dead squad group
(repelled, dead before a revert, a dead top-up, a dead HQ squad) go onto a
saved list (`Leftovers`). Each entry is checked every 30 s: a hull is
deleted when intact, unoccupied and `HullUnseen`; a group when all are dead
or its leader is beyond 850 m, and every body, living or dead, is unseen;
living stragglers go home room-capped, any shortfall booked "left behind".
A wreck stays. Budget: the list has no count cap and no per-step deletion
limit; its entries keep counting against the 40-group budget until
deleted, which is the cost the cleanup pays off.

**Withdrawal and break-off** (decision 4, the feel assessment's item 7,
2026-10-16) [code]. A live RETAKE, BASE or HQ_ASSAULT (`occupierForces` on
or a funded record) whose living infantry falls below a third of the
strength it was first born with (`BreaksOff`: alive x 3 < born; the born
strength is saved) while the target is still the resistance's breaks off,
on the road or at the edge; one man left of four is the same rule. It, the
live timeout and an HQ that moved all withdraw the same way, and at once:
the losses are booked, the record ends with one `CAOpRepelled` whose
`reason` is `CAERBrokenOff` or `CAERWithdrew` (a wiped force:
`CAERDefeated`), and the survivors become a withdrawing Leftover, ordered
home (their source, or the nearest occupier base if it fell) at every
30 s check and removed under the Leftover rule above. The live timeout no
longer waits for the player to leave: the squad walks off in the open.

**The hulls leave after the infantry** [code]. Infantry still seated rides
home in its carrier: the HomeHull carries the group, deleted unseen cargo
first, its living home as bodies, its dead lost; a carrier lost on the way
puts its living riders on foot as a withdrawing Leftover. Infantry on foot
is waited for: every hull holds where it stands (an armed one still
supporting, not driving, not deleted) until none of the survivors is alive
within `ClearRadius` of the target (the larger of
`counterAttackBirthMinToTarget` and zoneArea + 100 m: 250 m) or
`WithdrawHullWait` (120 s [code]) has passed, then drives home and is
refunded as before. A top-up squad whose town was lost (M5) keeps the same
two conditions on its hulls. Saved: the wait, the time held, the radius,
the group waited for, the cargo (`HomeHulls`), the Leftover's walk home.
Script surface: `gmCounterAttackEnds`, `gmCounterAttackHomeHulls`,
`gmCounterAttackLeftovers` (columns in `CounterAttackCommands.cpp`).

**One road factor** [code]. A ground leg with no road route known (no
road net, or a road past the detour rule) is timed and quoted with the same
factor: the theatre's `forcesRoadFactor` (1.5;
`ForcePackages::RoadFactor`, read through `CounterAttack::QuoteRoadFactor`
and `ForcePackages::GroundLeg`), the key-off legacy leg and the `speed`
test lever included. `counterAttackRoadFactor` (was 1.3) is a retired
legacy key: no longer read, a mission that authors it gets one WARN.

## 10. The pool (decision F)

### 10.1 Size, refill and bands

**Size** (OCCUPIER-FORCES.md 5.1). On the first active tick the service takes
its roster: every zone of a military type whose authored owner is the
occupier, independent of `campaignEnd`. R is the roster's summed
`garrisonMax`; K the administered, non-exempt CITY zones; G their summed
`garrisonMax`. `poolStart = max(forcesPoolMin, R + forcesPoolPerTown x K)`
(24, 2 [16]). An empty roster switches the service off for the session.
A SEEDED town (a map label, `seedCities`) counts in K and G only when it
passes the settlement probe: dry land and at least 3 house-sized buildings
within 300 m (OCCUPIER-FORCES.md 5.1, 16) [code]; authored towns always
count. The probe runs once, at the first active tick (or when a save
without the block derives), and its verdict is saved on the zone row
(`settlement`), so K is the same in every session [code]. A block saved
before the probe keeps its saved K. Sinai: K 26, poolStart 66 (37 and 88
before; 11 labels probed out); Abel unchanged (K 14) [measured]. An old
save's K counts by the AUTHORED owner, so towns the player already holds
stay in K (review M7) [code].

**Refill (OF D8, OF D15).** Per hour `(poolStart + R + G) / forcesPoolRefillHours`
(4 h [16]), accrued each director tick, scaled by the standing factor
`0.75 + 0.5 x meanStanding / 100` [16] (exactly 1 at the default 50). Whole
units enter the pool up to poolStart; only waves push it above. D2 regrowth
draws first, at most 1 body per 600 s per zone, so an exhausted occupier
fills its garrisons before its pool.

**Bands (hysteresis, shares of poolStart [16]).**

| Band | Enter | Leave | Effect |
|---|---|---|---|
| STRONG | > 66 % | < 60 % | a token per 1800 s; post operations, HQ assaults and push-out (towns and posts) allowed |
| STRETCHED | between | | a token per 2700 s |
| THIN | < 33 % | > 40 % | a token per 3600 s; no operation below `forcesPoolFloor` (20 %); alarm answers only with the cheapest package; garrisons stop chasing YELLOW contacts (`qrf.sqs`, also in SPENT [code]) |
| SPENT | 0 | > 10 % | no tokens; reverts left hollow (`Deposit` grants nothing); no alarm answers. D2 keeps drawing on the refill, so the pool sits near 0 while garrisons are short and the band leaves SPENT once, at 10 %, without flapping |

### 10.2 Spending and routing (OCCUPIER-FORCES.md 5.3 to 5.5)

| Path | Rule |
|---|---|
| D2 regrowth | `Draw`: min(bodies, pool); an empty pool grants nothing and clamps the zone's clock |
| Town or post refill on revert | the holding force's bodies settle first, up to the room; the surplus goes to the pool; the rest of the room is a pool-funded `Deposit`, pending inside the deposit bubble; SPENT grants nothing |
| Operation launch | the hull and crew price (both hulls of a column) through `Spend` (refused, nothing debited, over the pool or when `!CanOperate()`); infantry from the source base's STORED spare, granted only in full (M1); a foot downgrade refunds the whole hull price |
| Survivors home | to the source's reserve up to its room (pending inside the deposit bubble); the surplus to the pool |
| No base to return to | pool |
| Adopted live squad | settled once in `RevertTown`; the cut over the refill to the pool |
| A flipped base's live garrison | pool (key on); onto nothing else |
| Partial-spawn failures (garrisons, operations, crews) | pool |
| Surviving hull | price refunded when deleted at home unseen |
| Crew of a hull or airframe | priced as bodies (1 each) on the operation's ledger row, settled as bodies: home up to the room (the surplus to the pool), the dead lost, the living of a lost hull through the Leftover list. Each settle leaves the row by `CAOpCrewSettled`, crediting nothing, so a settled row reads 0 and a row pruned past `MaxOpRows` x 4 refunds only what is not crew still on the way home. A row from a save before this rule drops on load to what `CounterAttack::OwedOnOp` still owes it, crediting nothing (one INFO line) |
| Destroyed hull, dead bodies | lost (the `lost` diagnostic) |
| Hull deleted by a return to abstract travel | no refund; re-created on the next birth |
| Cancelled unborn operation | bodies to the source up to its room, the rest to the pool; hull and escort prices refunded (L2) |
| Re-sourced operation | the old base refunded up to its room, the surplus to the pool; the new base debited (L2) |
| Abstract REINFORCE arrival | into the target up to its room; the rest home through the room-capped return |
| Live top-up squad (REINFORCE or alarm answer) at its town | joins up to its room (`garrisonMax` - live - reserve - pending): the stored reserve, then the pending deposit, give way to it (to the pool); the surplus splits off (`CounterAttackOps::SplitOff`) and walks home as Leftover stragglers, removed unseen and home up to the room |
| Leftover stragglers | home up to the room; a shortfall booked "left behind" |
| Pending deposit, zone lost | key on: the pool (the refund hook); key off: back to the origin up to its room, else discarded [code] |
| HQ_ASSAULT on `hqLost` | an abstract record's bodies to the pool; a live squad leaves unseen and comes home |
| Landed wave | its bodies into the pool, above poolStart if need be |
| Legacy `Fight` win (key off only) | winners lose `min(A_bodies - 1, ceil(A_bodies x holdOdds x repelLoss))`; the rest settle |

### 10.3 Conservation

With nothing live,

`pool + reserves + pending + op strength = poolStart + R + G + refill + waves - losses - dropped`

**The ceiling** [code, round 3 C]. Every path in 10.2 that credits the pool
goes through `OccupierForces::Refund`, capped at poolStart plus the wave
headroom (what landed waves put above poolStart, shrunk at each refund to
the pool's overshoot plus the units the ledger rows still hold plus the
refund). The excess is banked as garrison (owner, 2026-10-16): a pending
deposit (S5, so it joins the reserve unseen or once the player has left)
at an occupier-held military base with room below its `garrisonMax`, the
source base first, else the nearest to the credit, else the one with the
most room, one INFO line `ledger: N unit(s) over poolStart banked at
<base>` per base. Banking moves units from op strength into `pending`, so
the equation needs no new term. Only what no base has room for is dropped
(`dropped`, saved) with one INFO line `ledger: N unit(s) over poolStart
dropped`. Only a landed wave puts the pool above poolStart
(OCCUPIER-FORCES.md 5.5).

holds after every path in 10.2. The unit tests (`[occupierforces]`,
`ConservationCheck`) assert it. U1 to U5 hold with the key on or off.

### 10.4 Default and old saves

**The default** [phase 7, landing with this commit]. `occupierForces`
defaults to 1 in the engine (`OccupierForcesTuning::KeyDefault`, read by
the service, by `CounterAttack` and by the `baseRetake` fallback), so
every Guerrilla template runs the layer: `Guerrilla.Abel` still authors 1
as the explicit opt-in (and `Showcase.Abel` inherits it through its
include), and `Guerrilla.Eden`, `Guerrilla.Sinai`, `Guerrilla.Lebanon80`
and `Guerrilla.Demo` author nothing and now run with the key on. Missions
with `campaignEnd = 0` pin it off by authoring `occupierForces = 0`: the
reference slices `Qrf.Abel`, `Market.Abel` and `Undercover.Abel`, and the
test fixtures (`guerrilla_offscreen.abel`, `guerrilla_native.abel`, and
from phase 7 `guerrilla_capture.Demo`, `guerrilla_persist.Demo` and the
rest). (Doc 14's "defaults on, `campaignEnd = 0` missions pin it off" is
read as "pin = author the key"; the engine does not derive the key from
`campaignEnd`.)

**Old saves** (OF D20) [commit bcca80e]. The forces block saves `format` 6.
A save whose block is older than format 6, or a save without the
`GuerrillaOccupierForces` block loaded with the key on, sets `legacySave`,
which makes `gmLegacyQrf` true: `qrf.sqs` keeps fielding its own script
QRF there and nowhere else, and native alarm answers stay off beside it
(row 7). A save without the block also derives the rest: the pool as
poolStart x C, crossed wave thresholds marked fired, the towns held
seeding the distinct set, tokens at 0 (so a migrated save does not launch
twice in its first minute). The old free QRF and fast heat decay come from
the script text the save carries and stay until a new campaign.

### 10.5 Persistence

- `GuerrillaOccupierForces` (gated on `HasState()`): pool, poolStart, R, K,
  G, the roster and excluded towns, `_townsEver`, posture, tokens and their
  clock, urgency latches, RED payments, cooldowns, the director and refill
  accumulators, the op counter and op ledger, the band, wave state and
  surges, the sightings (zone, count, at most 12 positions [code], stamp),
  `lost`, `legacySave`; `format` 6 since `bcca80e`.
- Zone rows: pending deposits, `occCapture`, `standing` (-1 = absent in
  an old save), `struckUntil`, `lostCount` [code].
- `GuerrillaBase`: `hqDestroyed`, `hqEpoch`, `hqCapture` and the queued
  `Ruins` (absent = not destroyed, epoch 0, meter 0) [commit c825a5f].
- `GuerrillaCampaignEnd`: the win latch, the lose latch (`lost`,
  `lostDay`, `lostMinute`) and `OnceHeld` [code].
- `GuerrillaCounterAttack`: records with kind, origin, package, op id,
  price, `destPos`, `hqDone`, the escort hull (class, crew, price, hull and
  crew-group links) and the `Leftovers` list [commit 6e1ee27, 37e24ba];
  since `bcca80e` also the records' air fields, the `AirHomes` list
  (airframes flying home) and `OccupierAir`'s insertions and sorties, whose
  hull, crew, cargo and pad refs resolve on the second pass (a pad missing
  after a load is re-created). `guerrilla_forces_air_save_reload.seq`
  resumes an insertion saved mid-flight.
- Zone-event handler slots `townLost`, `baseLost`, `hqLost` and
  `townStruck` [commit bcca80e].

## 11. The HQ, the win and the lose rule

### 11.1 The HQ (decision H; OF D13, D14, D22, D24) [code, `c825a5f`]

- **One HQ model.** `GuerrillaBase` is the only source of truth. With no
  START TOWN picked (or a failed pick) the first active tick of a campaign
  (`campaignEnd` on) establishes the HQ at the first CAMP zone, tried once
  per session and never after a destruction (`TryAutoElection`,
  `WantsDefaultHq`); the `campaignEnd = 0` fixtures keep no HQ at boot. An
  old save with no HQ gets the same on load unless its HQ was destroyed.
  A START TOWN HQ keeps the #79 B2 garrison exemption (tied to the pick); a
  re-established HQ never adds one (an accepted deviation). `HasHq()` is established and not destroyed.
  A retaken Camp does not become the HQ again: only an establish action
  makes one. The recruit anchor follows `gmHqPos`; no HQ, no recruiting.
- **Deliberate assault only.** The HQ meter (row 2d) counts only HQ_ASSAULT
  units within 150 m of `HqPos`. Passing patrols, columns and QRFs never
  drain or capture the HQ, wherever it stands.
- **No suppression.** The zone the HQ stands in, held or not, keeps its
  ordinary census, defenders and rows. A patrol may revert the town the HQ
  stands in while the HQ itself stands; an HQ inside an enemy zone does not
  make that zone easier to take.
- **Destruction at 100.** The weapon cache is emptied at once; its holder
  and the prop are queued as ruins and deleted once the perception gate rates
  them hidden (`SweepRuins`); every vehicle locked in the garage is destroyed
  (unlocked, damage 1, wrecks left where they stood) and the ring removed; the recruit
  point goes; `_hqDestroyed` is set, `_established` cleared, `_hqEpoch`
  bumped; one INFO line and the `hqLost` event. The zone keeps its owner.
- **Re-establish anywhere.** The existing `gmHqEstablish` action, in any
  zone, held or not, is free (a first establishment, not a move): an empty
  cache, a fresh ring, `_hqDestroyed` cleared. `market.sqs` watches
  `_hqEpoch` to drop and re-place the marker and re-arm the objective.
- **Before the director.** The legacy dispatch never targets the HQ's town
  (N4a, landed in phase 1), and no HQ_ASSAULT exists while DORMANT, so the
  opening can never cost the HQ.

### 11.2 The win and the waves (decision W; OF D7)

- **Win.** The roster is taken once, on the first active tick: every
  military zone whose authored owner is the occupier, plus every generated
  base (OCCUPIER-FORCES.md 5.7), minus the minor posts (a zone whose
  `countsForWin` is false, or of type CHECKPOINT, COASTAL, HELIPAD or RADAR;
  owner decision 2026-10-16: they capture, source and alarm, but do not count
  toward the win). `CampaignEnd` latches the win when every roster zone is held at
  once, checked every second; the latch is saved and never clears. After
  `campaignEndDelay` (10 s) the world's end mode is set to END1 once. Once
  latched, `OccupierForces` and `CounterAttack` freeze: nothing launches,
  nothing in flight steps, nothing reverts.
- **Waves** [commit 6e1ee27]. Control `C = 0.6 M + 0.4 T` [16] (M the roster
  strength the occupier still holds, T the administered share of the K
  towns). A trigger is due at C <= 0.75, then <= 0.50, and for the final
  stand when one roster base is left [16]; it fires after 2 consecutive due
  ticks; each fires once; triggers due together, or while a wave is inbound,
  merge. Size `forcesWaveShare x poolStart` clamped to 12..48; it lands after
  600 s (120 s for the final stand) [16] and adds its bodies to the pool and
  1 to the occupier war level. Allocation, in order [commit 6e1ee27]: the hub (the named
  `forcesWaveHub`, else the occupier-held AIRFIELD, else SEAPORT, else the
  strongest base) topped up by a pool deposit; the final-stand surge of up
  to 12 above the last base's strength, never spare, and the last base never
  sources a strike (the surge is excluded from spare only in the director's
  source choice, an accepted deviation); strikes on the best cluster (each
  held zone scores the sum over held zones within 2000 m of 4 for the HQ,
  3 for a base, 1 for a town, plus heat / 40 [16]), the HQ first if it is
  in the cluster, then bases, then towns, up to half the wave on up to 4
  targets (origin `CAOWave`), launched only once AWAKE (DORMANT logs "no
  strikes"); REINFORCE columns on foot from the hub to the most-bled held
  bases, at most 12 bodies each and `forcesWaveOps` columns; the rest stays
  in the pool. Each living wave record raises the born-operation cap by one
  (`CounterAttack::OpsCap`), and wave operations take no command token
  [code]. "In a wave", for the HQ candidate, means a wave inbound or landed
  within 1800 s (an accepted deviation). With no hub left the strikes
  depart from the occupier base nearest each target on that target's
  landmass [commit bcca80e] and no REINFORCE column goes (they need a hub)
  [code]. Wave strikes buy no air opener (an accepted deviation of phase 6).

### 11.3 Lose (decision L; OF D10, D13) [code, `c825a5f`]

- **Detection.** Checked every second after the win check, never once won,
  absent with `campaignEnd = 0` (`CampaignEnd::EvaluateLost`). LOST when
  the player has no HQ (`!HasHq()`) and the resistance owns no zone of any
  type (`HoldsAnything`: towns, posts and the Camp alike; the HQ's own zone
  counts only if the resistance owns it), AND the campaign "had something
  to lose": an HQ was ever established (`HqEver`: established, destroyed or
  epoch > 0) or a zone was ever held (`OnceHeld`, saved). A campaign that
  never had either is not lost (doc 9.2 is silent; the code wins).
- **Two ways to lose.** The HQ is destroyed while he holds nothing; or he
  loses his last base or town while he has not re-established an HQ.
- **End path.** One INFO line; after `campaignEndDelay` the end mode is set
  to `EMLoser` once, only while still `EMContinue`; the debriefing reads the
  lose title and body from `CampaignEnd::DebriefingText` (no `EMEnd2`
  fallback, which would play `OutroWin` and count as a completion); then
  `OutroLoose` and the main menu. The latch is stored only in
  `GuerrillaCampaignEnd`.
- **Saves stay.** GUERRILLA > Continue loads the latest save; a save taken
  while doomed reloads into the same loss.
- **Freeze.** `OccupierForces` and `CounterAttack` freeze on the lose latch
  as on the win.

## 12. Air (landed in `bcca80e`)

The spike's verdict was GO (`AIR-SPIKE.md`, `c5cfa44`): 18 of 18 insertions
on the final build landed within 100 m of the LZ (worst 20 m, median
5.5 m), unloaded 8 of 8 alive and departed; 6 of 6 jet sorties passed.
Phase 6 wired `OccupierAir` into `CounterAttack` (`CounterAttackAir.cpp`)
and the director [commit bcca80e]. The contract between them is
`CounterAttackOrders.hpp`: the kinds `CAKStrike` and `CAKAlarm`, the order
fields `airOrigin`, `strikeZone` / `strikePos` and `alarmZone` /
`alarmPos`, and the events `CAOpAirborne`, `CAOpLanded`, `CAOpStruck` (once
per strike) and `CAOpAlarmAnswered`. An insertion is an ordinary record
(RETAKE, BASE, HQ_ASSAULT, REINFORCE or ALARM) with the transport
helicopter as its package. Row 25 states the off-screen rules in one place;
this table keeps the recipe. Ground packages never depend on air: a
faction that authors no aircraft (GUER, Hizballah, PLO) simply buys none.

| # | Rule |
|---|---|
| A1 | **Sources (OF D3).** Air comes from the occupier-held AIRFIELD nearest the target (`airOrigin`), else from off-map (`forcesOffMapEdge`, default the edge nearest the target, the point 1500 m beyond the map, level with the target) at 1.5 x the price [16] and a longer flight, fixed at launch [commit bcca80e]. A base on another landmass than the target is an air-only source (section 9.3). |
| A2 | **Birth (decision S at altitude).** Helicopters and jets are born airborne, never on the ground (a jet on the ground would taxi the world's single ILS path). The recipe [air, code]: orient and `SetTransform` first; `SetFlyingHeight(altAgl)` (only when above 0) BEFORE `MakeAirborne()`, which returns the height to add; `SetTransform`, `Init`, `AddVehicle`. Crew and cargo are two groups, crew CARELESS / GREEN, cargo SAFE. The gate (`AirViewer`, `ExposureAt`) is Traffic's camera, band and terrain ray evaluated at the aircraft's height; the audible floor for aircraft is `airAudible` (800 m [code]); a point beyond the effective band in 3D from the eye is hidden even inside the view cone. Off-map is rendered sea, not automatically unseen. |
| A3 | **Insertion (OF D20).** The LZ is chosen natively on the 300..500 m ring [16, code] around the target, the side away from reported fighters first; each candidate snaps to the centre of its 50 m terrain cell, which must pass the engine's landing test (no water, not full, no objects, gradient below 4) plus no forest flag and a slope of at most 0.25 across 22 m. Never within 250 m of the target; the landing may be in plain view. An invisible `HeliHEmpty` pad (falling back to `HeliH`) goes into the BUILDINGS list (`AddBuilding`, because `FindStopPosition` walks only that list and only within 500 m), and `LandStarted(LMLand)` is ordered within `landOrderDist` (450 m) of it, re-issued every 5 s until down. Touchdown is height at most 5 m and speed at most 3 m/s for 1 s. A landing more than 100 m [16] from the LZ (or a timeout, or no pilot) is a failed insertion: the airframe flies home with its cargo and is removed unseen; the record goes back to abstract flight once (the airframe re-created at the next birth, no refund), then is cancelled unseen [commit bcca80e]. No LZ, or no airframe: the force goes on foot, its price refunded. Crew and cargo count 2 groups against the 40-group budget; the LZ side is chosen from the centre of the director's sightings (OF D9). |
| A3b | **Unload and departure** [air]. The cargo gets `OrderGetIn(false)`, `AllowGetIn(false)`, `DoGetOut`, then a per-unit `AIUnit::UnassignVehicle` (never the group's, which clears the crew's seats; without it the helicopter landed again 110 to 285 m away), and walks on the target. With no cargo seated: `LandFinished()`, the pilot set to Wait when Stopping or Stopped (without it a 30 m hover over the LZ), then a `Command::Move` home through `IssueCommand`, re-issued every 45 s while it stays down. The cargo group becomes the record's live squad once unloaded and walks the straight line from the LZ; the airframe goes onto the record's `AirHomes` list [commit bcca80e]. The pad no longer goes at departure: it goes with the hull (A4). |
| A4 | **Removal** [commit bcca80e]. The airframe flies home and is deleted only where it is unseen for 2 s (`removeHold`), beyond the map edge included: the crew FIRST (with any cargo still seated; each man leaves his seat before he is deleted, the `EngineOps::LeaveVehicle` idiom, so none lingers in the world's out-vehicle list or reaches a save, row 23), the hull one step later, the pad with the hull. **Unseen for an aircraft** (the owner's decision for phase 6, applied): `OccupierAir::UnseenAt` accepts a point BEYOND the perception band in 3D from the camera even inside the view cone (nothing is drawn there), or out of the view cone and terrain-masked at altitude, never inside the 800 m air audible floor; `departTimeout` (900 s, beyond the band) stays as the backstop. Home unseen, the price is refunded and the crew go home as bodies (`CAOpHullRefund`); a destroyed airframe is lost (`CAOpHullLost`). |
| A5 | **Strikes (OF D17, OF D9)** [commit bcca80e]. A `CAKStrike` record carries no bodies. Abstract, it reaches `strikePos` on its clock. Live (the target in the bubble, the budgets free) it is an `OccupierAir` sortie: a jet or attack helicopter born airborne (250 m for a jet) under the same gate, flying to the target (AWARE / RED) and, within 1.25 x the orbit radius, searching in COMBAT / open fire on an 8-point orbit 900 m out, a leg every 20 s, for `forcesSortieTime` (300 s [16], read from config); then home and removed as A4. Its own sensors pick targets; the limited reveal (`AIGroup::AddTarget`), every 10 s, covers only sightings younger than 60 s, resistance AI within 100 m, at accuracy 0.5, never the player [air, code]. `CAOpStruck` goes out once, live or abstract, and the director applies row 21. The spike proved birth, search, reveal, departure and removal, not damage on a target; attack helicopters share the jet's path. |
| A6 | **Town strikes (OF D21).** Row 21. |
| A7 | **Alarm answers by air (OF D4).** The director may answer an alarm (row 7) with an insertion; the transport helicopter joins the alarm package choice like any other [commit bcca80e]. |
| A8 | **Air re-plan.** Step 7 of section 9.3. |
| A9 | **Not built (accepted deviations of phase 6).** No attack-helicopter escort for an insertion (an insertion carries one airframe); strikes are live only in the bubble; wave strikes buy no air opener; an HQ strike with no report aims at `HqPos`; a base-opener strike cuts only the 2 km ring. |

## 13. Changelog against the current contract

**Dropped.**
- Decision R's "liberated MILITARY zones are permanent in v1" as the rule
  (kept only as the `baseRetake = 0` behaviour and a superseded note).
- Decision C's "every town from k >= 1 is answered with its own force" as
  the rule with the layer on (kept as the key-off and DORMANT dispatch,
  section 9.1, with a superseded note).
- The sentence "An occupier forces pool, a standing retake director and
  reinforcement waves are being designed separately" (they are built).
- The #38 follow-up "occupier recapture of liberated MILITARY zones" (row 1b
  covers it).
- The working-tree tag [wt] (pass of 2026-10-14).

**Changed.**
- Decision D1: held towns and posts also tick off-screen with occupier
  presence. D2: draws on the pool. B: names the token, born-op and hull
  budgets, the column's 3 groups and 2 hulls, and the Leftover cleanup.
- Row 1 split: the player's meter stays in row 1, the base recapture moves
  to a new row 1b (the committed contract folded it into row 1).
- Row 2b: the occupier census is stated in full (OF D5) and OF D18 added.
- Row 2c: from "counter-attack on a risen town" to "occupier operations" of
  every kind, with key-on live resolution, no odds roll, the edge-hold
  give-up, the hull gate and the Leftover list (the review found the old
  "no entity created or deleted" false with the key on).
- Row 4: S5 restated as a hard rule with its tests and the routed writes;
  the pending refund on owner change goes to the pool with the key on.
- Rows 7, 8, 9, 10, 11, 15 and 16: alarm answers, the pool as the occupier's
  supply model, held-zone heat decay and the +15 repel heat (the review
  found the old row 9 false), the occupier war level, urgency, and the
  recapture summary.
- Section 2: the always-live class and the persistent state list now include
  operations, hulls, escorts, the Leftover list, the HQ and the end latches.
- Section 4: split into legacy and occupier-forces keys; legacy keys note
  where the key-on path ignores them.
- Section 5: C8 states the road-route join and the Compound's open gate
  stall.
- Section 6: new budget rows and every forces instrument with its committed
  layout; the worst case notes what the layer adds.
- Section 7: the forces tests, all landed, the lose test at its real path,
  the armoured column and the two `slow_air` spike tests.
- Section 9: restructured into the legacy dispatch, the director, unseen
  births (a numbered hard rule with the full chain, the road-route join and
  the edge hold), accidental retakes, saves and seams, and columns.
- Section 8: #72's partial-spawn leak closed with the key on; THIN / SPENT
  YELLOW chase; the review items still open.

**Added.**
- Decisions S, S5, P, H, L, W, F, A, U (U was the unnumbered "these rules
  apply to every mission" paragraph).
- Rows 1b, 2d, 18 to 25 (23: removing a seated man; 24: the Leftover
  cleanup; 25: occupier air, from the swap); coverage target C8 (road nets
  join).
- Sections 9.6 (columns and the hull lifecycle), 10 (the pool), 11 (HQ,
  win, lose), 12 (air), 13.
- Number source tags and "(doc says X)" notes.

**Fixed.** Row 17 had been orphaned below the "these rules apply to every
mission" paragraph, outside the table; it is back in the table and the
paragraph is decision U. The `weaponSeen` line of section 6 is true again
since the Undercover TIME_MIN fix (`UndercoverWeaponSeenStampCurrent`,
`6e1ee27`; review H1).

**This pass (2026-10-14), against `c5cfa44`.** Every [wt] rule retagged
[code] or [commit X]; rows 1b, 2c, 2d, 4, 20, 22 and decisions B, R, C, S,
H, L, W, A brought to the committed code; rows 23 and 24 and section 9.6
added for phase 5; section 9.3 rewritten for the immediate re-source with
a route check, the road-route join, the edge hold and the key-on legacy
chain; section 12 aligned to AIR-SPIKE.md with the 3D-band unseen rule for
aircraft; instruments given their committed layouts; the disagreements
below resolved.

**The swap (2026-10-14), against `bcca80e`, phase 7B.** This file replaced
the tracked contract, and the draft file was deleted. Folded in from the
tracked contract, where the phase 6 agents had written them after the
draft: row 7 (QRF and alarm answers, `gmLegacyQrf`), row 8 (half income
while struck), row 13 (`townStruck` panic), the tracked row 17 "Air
strikes on towns" as row 21 (the draft's row 17 is the always-live actors
and its row 21 held the pending town-strike rule), the tracked row 18
"Occupier air: births, LZ, removal" as the new row 25 (the draft's row 18
is the pool), and the `forcesSortieTime` / `forcesOffMapEdge` key rows.
Every [phase 6] and [phase 6, pending] rule turned into the landed rule:
decisions S and A, rows 2c, 9 and 22, the air packages and the air re-plan
(section 9.2, 9.3 step 7), budgets and instruments (29-slot
`gmCounterAttacks` and `gmOccupierForces`, `gmZoneStruck`, `gmLandmass`,
`gmGeoGrid`, the new `gmForcesForce` levers), the four new tests, the
persistence of air and `format` 6, and section 12 (A1, A3 to A7 rewritten,
A9 added; `departTimeout` stays 900 s, which the draft expected phase 6 to
lower). Added: the landmass rule (section 9.3, decision C, C8, rows 5 and
25), closing the Compound's gate stall. Row 23: Traffic (`2524c3a`) and
`OccupierAir` now move seated men out first. Decision F and section 10.4:
the phase 7 default, [phase 7, landing with this commit]. Section 8: the
phase 6 leftovers and accepted deviations.

**Disagreements, resolved: the code wins.** Each entry says what the code
does, and which OCCUPIER-FORCES.md place phase 7 edits (section numbers;
line numbers as of `c5cfa44`).

1. **K and the settlement probe. STILL OPEN for phase 7.** The engine
   counts every administered non-exempt CITY; no seeded table, no probe
   (`OccupierForces::Activate(registry)` [code]). Doc 5.1 (lines 428 to
   433) and 12.1 (line 1197, "K with the probe"). Phase 7 decides: run the
   probe, or drop it from the doc.
2. **Pending deposit on an owner change. RESOLVED** (`6e1ee27`): the pool
   with the key on, the origin or discard with it off. Doc 3.3 (lines 380
   to 382) already says so; the plan's phase 1 status line stays as
   history. No edit.
3. **Re-sourcing. RESOLVED** (`6e1ee27`): immediate, to the next base whose
   stored spare covers the force and whose route passes the gate; wait only
   when none passes; after `counterAttackBirthGiveUp` once more, then
   cancel unseen. Doc 3.1 step 3 (lines 289 to 292): add "at once", "whose
   stored spare covers the force", "whose route yields a point passing the
   gate (the road A*, else the straight line)", and the wait-then-retry
   before step 5. Doc 16 line 1401 (`counterAttackBirthGiveUp`): "key on:
   the wait when no base passes, then one more re-source, then cancel".
4. **Legacy records. RESOLVED** (`6e1ee27`): with the key on they take the
   chain and never turn back (OF D6). No doc edit; the 16 line 1401 note
   above covers key off.
5. **`occCapture` under resistance-only presence. ACCEPTED DEVIATION:** it
   decays at `captureDecayDefended` (10). Doc 7.2 line 833 ("Frozen.
   Resistance presence freezes it") must read "Both present freeze it;
   resistance alone decays it at `captureDecayDefended`"; doc 16 line 1386
   likewise. Also 7.2 line 838 to 839 and 8.1: every holding abstract force
   counts as occupier presence, of any kind, not only BASE and HQ_ASSAULT.
6. **The default. RESOLVED in phase 7** [phase 7, landing with this
   commit]: the engine default is 1 (`OccupierForcesTuning::KeyDefault`),
   and `campaignEnd = 0` missions author 0 (section 10.4). Doc 14 and 16
   (the `occupierForces` row) to read so.
7. **HQ final push. RESOLVED as the code says:** unconditional when the
   resistance owns no post and no town other than the HQ's own zone
   (`FinalPush`). Doc 6.3 lines 602 to 606: "no base or town" becomes "no
   base and no town besides the HQ's own zone"; also "in a wave" = inbound
   or landed within 1800 s, and an HQ outside every zone is never a heat
   candidate.
8. **Waves and the born cap. RESOLVED:** each wave record raises
   `forcesOpsMax` by one (any phase) and wave operations take no token;
   the surge is allocated after the hub top-up and before the strikes;
   with no hub, strikes leave from the nearest base and no REINFORCE goes.
   Doc 6.3 line 612 ("At most 2 born operations"), 7.1 lines 807 to 822
   (allocation order, tokens, cap, the no-hub case), 6.6 line 781
   (`gmForcesForce` gains "cooldown").
9. **Informant tips read the true census. ACCEPTED, D9-legal per 7.3:** the
   town's true HOLD count, jittered up to 100 m per axis. Doc 6.1 lines 564
   to 567 ("Never used: the true census") gains the informant exception.
10. **The lose latch needs "had something to lose".** `HqEver` or
    `OnceHeld`; "holds" means owns any zone of any type. Doc 9.2 lines 1081
    to 1084: add the clause.
11. **The lose test** is `ui/remaster/remaster_ud_guerrilla_forces_lose`.
    Doc 9.2 line 1114 and 12.2 line 1287: rename.
12. **Hull packages and `forcesOpsMax`. RESOLVED** (`37e24ba`): a hull
    package downgrades to foot at launch with the whole price refunded;
    `forcesOpsMax` has one source (the `OccupierForces` tuning). Doc 6.5
    lines 686 to 691 already describe the downgrade; the phase 3 status line
    keeps its "known gaps" as history (the phase 5 row records the fix). No
    edit beyond 16 line 1367 (note the single source and the wave
    increment).

**Further plan-doc places phase 7 edits (found in this pass).**
- 3.1 line 280 to 282 and 314 to 316, 3.2 lines 345 to 346 and 354 to 358:
  the air recipe (`HeliHEmpty` with `AddBuilding`, the 450 m landing order
  re-issued every 5 s, per-unit unassign, `LandFinished` plus Wait), the
  800 m air audible floor, and removal "unseen" accepting beyond the band
  in 3D with `departTimeout` as the backstop.
- 3.2 lines 336 to 337: the column (escort 20 m behind, carrier waits past
  150 m, drives on inside 60 m); the dismount after braking (up to 15 s,
  so possibly inside the bound).
- 3.1 (new step): the road-route join (stage 2, within 2000 m of the source,
  best-cost landing, only on the source's landmass) and the landmass rule
  (phase 7B edits 3.1 and 3.2 for it); 12.2 (`guerrilla_forces_watched_route`
  asserts the landmasses and the re-source to the Airfield since
  `bcca80e`).
- 5.5 lines 513 to 526: add the room-capped re-source and cancel refunds,
  the abstract REINFORCE arrival, the Leftover stragglers, the pending
  refund on an owner change, the `hqLost` settle; note the live top-up
  adoption has no room cap (open).
- 6.5 line 710: the base retake minimum is capped at 12 in the code.
- 6.6 lines 777 to 778: `gmOccupierOps` has 13 columns (hull and escort
  classes), `gmOccupierForces` 29 slots since `bcca80e`.
- 11 lines 1150 to 1171: `Leftovers`, the escort fields, `lostCount`,
  `OnceHeld`, `Ruins`; since `bcca80e` the air fields, `AirHomes` and the
  `OccupierAir` records are saved (no longer transient).
- 12.2 lines 1290 to 1291: add `guerrilla_forces_armoured_column` and
  `guerrilla_forces_jet_sortie`; the heli insertion test is the spike's.
- 16: `forcesHqAssaultMin` and `forcesControlDwell` by name (1..10),
  `forcesLiveHullsMax` owned by CA, hull override keys read from the
  faction descriptor, `forcesSortieTime` and `forcesOffMapEdge` (read
  since `bcca80e`), `airAudible` 800 m, `landOrderDist` 450 m,
  `departTimeout` 900 s,
  `EscortGap` 20 / `EscortSpread` 150 / `EscortRejoin` 60, the join radius
  2000 m, the hull gate (night above 0.2, 2.5 m ray).
