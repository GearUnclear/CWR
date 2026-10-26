# Occupier forces: the strategic pool, the director and reinforcement waves

**Status.** DESIGN, round 5 (final), 2026-10-13. The owner's decisions
(section 0.2, D1-D20) are binding and override anything older.

**In the code already.** `4400fe9` (the first-town curve) and `450327e`
(counter-attack forces born out of sight, marching from their base), plus the
phases marked landed below.

**Implementation status** (branch `occupier-forces`, updated as phases land):

| Phase | Status | Notes |
|---|---|---|
| 1 | Landed | U1 to U5 (pending deposits on the zone row, 3x3 hidden-release grid, phase-1 refunds go to the origin post or are discarded), any-enemy census, HOLD and CAPTURE predicates (`gmPlayerPresence` slots 6 `holds`, 7 `weaponSeen`; `gmUndercoverWeaponSeen`), `GuerrillaBase::HqZone()` (falls back to the establishing zone when the HQ sits just outside its 150 m area, instead of -1) with the legacy dispatch skipping the HQ town. `guerrilla_town_defenders` t4 to t6 and `guerrilla_counter_attack_watched_base` updated for S5. |
| 2 | Landed | `OccupierForces` core: roster, K, pool, refill, bands, control, war-level split, `GuerrillaOccupierForces` block with old-save derivation, `gmOccupierForces` / `gmForcesTune` / `gmForcesForce`, `guerrilla_forces_pool_off` / `_on`. Inert with `occupierForces = 0` (every pool call passes through). Not yet wired: D2 `Draw`, the pending refund hook, the 5.5 routing rows (phase 3). Measured on Abel: garrisons full at about 2.57 h, not 2.38 h, and the band leaves SPENT (pool past 10 %) just before the last short garrisons fill, once, with no flapping; 5.2 and 5.6 below are corrected to say so. |
| 3 | Landed | Director in `OccupierForces` (sightings-only intel per D9 at the alert cadence, `gmOccupierIntel`; DORMANT until the second distinct town; heat candidates with the per-flip roll, urgency with hysteresis, tokens by band, 30 min cooldowns, push-out; `gmOccupierOps`), `ForcePackages` (ladder classification with the seven descriptor overrides, prices, lethality, the choice with both worked examples; phase 3 offers Foot and Truck), real speeds, `gmHeatDecayHeld` 0.25. Movement in `CounterAttack`: record kinds and fields, `LaunchFunded`, the listener events (`CounterAttackOrders.hpp`), truck hull lifecycle (crew group, cargo, `IssueCommand` driving, brake and dismount at 250 m, delete without refund on a return to abstract travel, refund when home unseen), the birth chain with next-base-back re-sourcing and cancel-unseen for funded records, key-on live resolution against present defenders, the 5.5 routing rows, the won freeze, `ZoneRecord::seeded`. Abel opts in; the fixtures and reference slices pin the key off. Known gaps: Malden's Southeast Compound road net is not joined to the island network (A* exhausts it), so `guerrilla_forces_watched_route` asserts the re-source and the departure, not the arrival; `forcesOpsMax` is read by both services separately; a hull package needs stored spare at its source (no foot downgrade). Closed 2026-10-16 (round 3.5, video 4): arrival-live births were on foot with the hulls held back; they are now born with the carrier, the escort and their crews like a route birth (6.5 "At the town edge"). |
| 4 | Landed | Commits c825a5f (4B) and 6e1ee27 (4A); gate green 2026-10-13 (626 unit cases, 33/33 tri on shadow-67, four one-off flakes). 4B: the HQ model and the lose state, on disk and verified, not committed. With no START TOWN a campaign (campaignEnd on) establishes its HQ at the first CAMP zone through `Establish` (once per session, never after a destruction; `campaignEnd = 0` fixtures keep no HQ at boot; an old save with no HQ gets it on load). `HasHq`, `HqDestroyed`, `HqEpoch`, the HQ meter `hqCapture` at `HqPos` (only `CounterAttack::HqAssaultPresence` fills it; resistance HOLD presence within 150 m freezes it; decay otherwise), `Destroy` (cache emptied, holder and prop deleted once Hidden, garaged vehicles unlocked and damage 1, ring dropped, epoch bumped, INFO line, `ZEHqLost`), free re-establish anywhere; `gmHqState`, `gmHqEpoch`, `gmHqForceCapture`; recruit menu on `gmHqPos` (none without an HQ); `market.sqs` marker and objective re-arm on an epoch change. `CampaignEnd` lose latch (no HQ, no held zone, and an HQ or a held zone once), `EMLoser` after `campaignEndDelay`, the lose debriefing in `UIMapDialogs`, `gmCampaignLost`, `gmCampaignEnd` slot 8, all saved. Deviation: a start-town HQ still gets the pre-existing #79 B2 garrison exemption (tied to the start-town pick, not to the HQ). Tri green: `guerrilla_forces_hq_assault`, `remaster_ud_guerrilla_forces_lose`. 4A: waves, base recapture, standing and the director's base and HQ operations, on disk and verified, not committed. `ZoneRecord` `occCapture`, `standing`, `struckUntil`, `lostCount` (saved); `RevertBase` and the base branch of `EvaluateTick` (any occupier unit or an abstract BASE / HQ_ASSAULT force raises `occCapture` by `captureRate` x min(crew, 3) with no resistance HOLD presence; both present freeze it; nobody decays it at `captureDecayAbandoned`; at 100 the post reverts, the holding force settles, else a pool deposit, income 0, `baseLost`; never after a win or loss); `gmZoneSim` slots 14 to 17, `gmZoneSet` `occCapture` / `standing`; `capture.sqs` per-zone hold groups, "Retaken" by `lostCount`, the `baseLost` handler. CounterAttack: BASE, HQ_ASSAULT and REINFORCE launches validated per kind, `destPos` / `hqDone` saved, `HqAssaultPresence` (HQ_ASSAULT records only, D14), HQ records settle on `ZEHqLost`, `OpsCap` = `forcesOpsMax` + live wave records, the Won / Lost freeze. Director: base candidates only while STRONG (D16), bases in push-out, the HQ candidate while STRONG or in a wave, the final push unconditional; waves (two-tick dwell, merge, size 12..48, lead 600 / 120 s, hub AIRFIELD then SEAPORT then the strongest base, hub top-up, cluster strikes up to `forcesWaveRetakeShare` on `forcesWaveOps` targets, REINFORCE columns, final-stand surge); `OccupierWarLevel` (`gmWarLevelOccupier`) for garrison tiers, CounterAttack, Traffic, Undercover and `qrf.sqs`, which also skips the YELLOW chase in THIN / SPENT; standing (recovery toward 50 at 1 per 600 s, informant tips at `standing / 200` from 50, morale 0.8 below 25, the mean scales the refill; the strike that cuts it is phase 6, the hook is `ZoneRecord::standing` / `gmZoneSet "standing"`). Phase 3 gaps closed with it: the S5 pending refund goes to the pool with the key on (`OccupierForces` installs `GarrisonCache::SetPendingRefundHook`); a force with no unseen birth point is re-sourced at once to the next base whose road passes the gate, waits only when none passes, then is cancelled unseen; with the key on legacy (DORMANT) records take the same chain and never turn back (D6). Accepted deviations: the `baseRetake` key (follows `occupierForces` unless authored); resistance alone decays `occCapture` at `captureDecayDefended`; "in a wave" means a wave inbound or landed within 1800 s; the final-stand surge is excluded from spare only in the director's sources; a base route is the road A*, else the straight cross-country line, which counts as reaching the target. Tri green: `guerrilla_forces_base_retake`, `guerrilla_forces_wave_base`, `guerrilla_forces_refill_pushout`; `guerrilla_counter_attack_watched_base` now asserts the re-source (key on) and `guerrilla_forces_pool_off` the 24-slot row. |
| 5 | Landed | Every ground package offered (`DefaultPackages`). Column decision: armour escorts a carrier (truck or APC); one escort per column at most; a lone tank or two-seat armed car is never chosen (`seatsShort`), only bought as an escort. The escort is born `EscortGap` (20 m) behind the carrier under the same unseen rule, drives the carrier's legs (the carrier waits while the escort is more than 150 m behind), closes up to the town edge and supports there (armed carriers support too; a truck holds back); each hull refunds its own price when deleted unseen at home; a return to abstract travel deletes both hulls crew-first, no refund, re-created at the next birth. Launch-time foot downgrade refunds the whole column. Funded orders are granted from the stored reserve the director quoted, else refused (review M1); a hold at the town edge gives up after `counterAttackBirthGiveUp` into a town-edge birth or a cancel unseen (M2); lost hulls, crews and dead squads go to an unseen cleanup list, saved as `Leftovers` (M3, L7); hull births and despawns use the headlight band at night (M8). REINFORCE columns top up through the 5.5 room rows; a top-up squad whose town is lost walks home and leaves unseen (M5). `forcesOpsMax` is read only from `OccupierForces`. Tri: `guerrilla_forces_armoured_column`. |
| 6 | Landed | Commit bcca80e; gate phase 6 green 2026-10-14 (663 unit cases, 45/45 tri on shadow-67 incl. the slow_air tests, human suite OK). Also in it: the landmass rule (3.1: a ground force never comes from, and a Traffic vehicle never drives to, a base on another landmass; Malden's Southeast Compound is an island and an air-only source). 6A: air in CounterAttack on disk, not committed (CounterAttackAir.cpp; contract in CounterAttackOrders.hpp: CAKStrike, CAKAlarm, `airOrigin`, `strikeZone` / `strikePos`, `alarmZone` / `alarmPos`, CAOpAirborne / CAOpLanded / CAOpStruck / CAOpAlarmAnswered with `zone` and `pos`). Air packages join `DefaultPackages`, offered only where the descriptor authors `forcesTransportHeli` / `forcesAttackHeli` / `forcesJet` (EAST Mi17 / Mi24 / Su25, WEST UH60 / Cobra / A10, @LoBo IDF, Egypt, Syria, Jordan; FIA and the irregulars none); prices 10 / 18 / 22, x1.5 off map; the strike airframes carry nobody. Insertions: abstract on the straight line from `airFrom` (the airfield, else the `forcesOffMapEdge` edge, 1500 m out), born airborne at the first point the altitude gate hides, LZ 300..500 m away from the reported fighters, unload, the cargo becomes the live squad, the airframe home through `AirHome` and refunded once OccupierAir removed it crew first; a landing over 100 m off the LZ re-plans once, then cancels unseen; no LZ or airframe goes on foot. Birth chain step 4: a funded ground operation with no unseen birth point after the next base back is re-planned as an insertion bought from the pool (refunded directly, `airReplanned`). Strikes: abstract on their clock, live as an OccupierAir sortie only while the target is in the bubble, `forcesSortieTime`, limited reveal every 10 s of sightings under 60 s (resistance AI, accuracy 0.5); CAOpStruck once. Alarm answers travel to `alarmPos`, answer once, hold until below RED with no contact for 60 s, then join the garrison. Owner decision applied: aircraft are unseen past the band in 3D even in the view cone (`departTimeout` the backstop). Fixed in OccupierAir: crew leave their seats before deletion, the pad goes after the hull. CounterAttack's vanish and top-up removals now wait for the dead too (`BodiesUnseen`). Air records, `AirHomes` and OccupierAir's records are saved in the CounterAttack block. Tri green: `guerrilla_forces_air_op`, `guerrilla_forces_air_save_reload.seq`. Deviations: no attack helicopter escort for insertions; a live alarm answer adopts its whole squad (no garrisonMax cap on the live path); the descriptor lint checks the `forces*` classes since phase 7A. 6B: director side on disk, not committed. Air buying: the transport helicopter joins every package choice when the catalogue offers it (`airOrigin` = the occupier-held AIRFIELD nearest the target, else -1 and the off-map price through `PackageRequest::airOffMap`). Strikes (7.3): bought only while STRONG, as an add-on right after a director ground launch on an URGENT held town with 3+ reported fighters outside its strike window, or on a base retake / HQ assault (the opener; launched just after the ground order, the jet arrives first), or alone on such a town when no ground package can go; jet or attack helicopter, the cheapest the pool pays, the fastest within `forcesUrgentCostShare` (always urgent); `strikePos` the centroid of the zone's reported positions (D9). Effects on every `CAOpStruck` (`ZoneRegistry::ApplyStrike`): -40 support / -40 standing / +15 heat / `struckUntil` = mission time + 1200 s in a struck CITY, -10 / -10 in every other CITY within 2 km of it (a strike on a base or a position: the ring only), `townStruck` queued (civilians.sqs panics through `GM_fnPanicStart`), `gmZoneStruck` halves the town's income and stops its manpower (economy.sqs). Alarm answers (8.2): any zone the resistance does not hold at RED, one CAKAlarm at a time at the 5 s scan cadence, sources every occupier base but the alarm zone, minimum 4, urgent unless THIN, none when SPENT ("Starved" once), `alarmPos` the latest report. qrf.sqs launches only while `gmLegacyQrf` (key off, sized off, or `legacySave`); the block saves `format` 6, an older block loads as `legacySave`. Deviations: alarm answers take no token and run while DORMANT (8.2 sets their limits; otherwise a key-on campaign has no answer before the second town); wave strikes buy no air opener; the HQ strike aims at `HqPos` when nothing was reported there. |
| Rate | On disk, not committed | 2026-10-15, the owner's "a more playable rate" (6.3, 6.5, 7.2, 16). Cadence: `forcesTokenInterval[]` {600, 1050, 1800, 0} (was {1800, 2700, 3600, 0}) and `forcesOpsMaxStrong` 3 (the born cap while STRONG; 2 otherwise), the arithmetic against the pool in 6.3. The base retake minimum: the spare cap (`forcesSpareCap` 1, floor `forcesSpareCapFloor` 6) instead of two-source launches, which CounterAttack's one-record-per-target model does not allow (7.2); the refusal line names the shortfall. The arrival rule: `forcesSlowEta` 900 s, `forcesSlowCostShare` 3.0, `forcesAirEta` 1500 s (6.5). Every old value stays reachable through its key (16). Unit: the `[occupierforces]`, `[packages]`, `[counterattack]` and `[air]` cases; tri: `guerrilla_forces_cadence`, `guerrilla_forces_spare_cap`, `guerrilla_forces_arrival_rule`. Section 5.6's band table and section 10's pacing rows still quote the old cadence; the "cadence at a glance" table in 6.3 supersedes them. |
| 7 | Landed | Commit fff2c0b; gate phase 7 green 2026-10-14 (664 unit cases; tri 75/80 on shadow-67 where the five reds are the main-menu lane's --mods-dir .. spawn failures on a tmp shadow and the pre-existing mp_wizard_template_overview, every forces / off-screen / reference / remaster test green; human suite OK). Defaults on (7A: `occupierForces` defaults to 1 in the engine, `OccupierForcesTuning::KeyDefault`; `campaignEnd = 0` missions author 0; the descriptor lint checks the `forces*` keys), the contract rewrite (7B: OFFSCREEN-CONTRACT.md replaced by the reconciled draft with phase 6 folded in; these docs), the fork issue, the roadmap, the PR. |
| Bases 1 | On disk, not committed | 2026-10-16, round 4 phase 1 E: generated bases step 0 and step 1, zones only (5.7). `BaseGenerate` (the density rule, the scorer, the stored `GeneratedBases` rows), `generateBases` on in Abel and Eden, the detected OUTPOSTs as preferred sites, `countsForWin` and the minor kinds in `CampaignEnd`, `gmGeneratedBases`. Abel gains two OUTPOSTs and closes all seven gaps; Eden gains two and leaves Regina open. Conservation is conservation as decided: generated garrisons (Rgen) come out of poolStart, waves stay on poolStart + Rgen (phase 1 A) until `OccupierForces::Size` subtracts the generated garrisons from poolStart. Unit `[basegenerate]`; tri `guerrilla_bases_generate_{abel,eden}`, `guerrilla_bases_generate_save.seq`. Props are step 2. |

**Reading guide.**
- Section 16 is the internal knob reference.
- Section 17 holds what is still open.
- Section 18 is the decision and critique log.
- Section 19 lists the residual risks.

This is an AI-drafted design document: where it and the code disagree, the
code wins and this file gets fixed.

## 0. The brief

### 0.1 What the user asked for

1. **Answer town flips, present or not.** A force answers the taking of
   towns whether or not the player is there.
   - After the first town there is about a 40 % chance of an immediate
     answer.
   - From the second town on, heat and the retake systems take over, and
     both towns can be retaken.
2. **A pool, not one force per town.** A forces pool replenishes the
   occupier. It does not send one force per town. When the occupier is
   really losing control, a parent nation sends reinforcements that arrive
   all at once.
3. **Heat, alert and war level are part of the design.**
4. **Hard rule.** Soldiers always navigate from a base where their spawn is
   not seen by the player to the town they are retaking.

### 0.2 Owner decisions (binding)

| # | Decision |
|---|---|
| D1 | The director picks foot or motorized per operation. A truck costs more but raises lethality; the director weighs a lethality index against cost. |
| D2 | It buys anything it can afford, priced in pool units, with no war-level gate: trucks, armed jeeps, APCs, tanks, helicopters, jets. War level stays a quality and heat signal. |
| D3 | Air: helicopters do troop insertion (fly in, drop soldiers, leave); jets fly too. Air comes from an occupier-held airfield, else from off-map, which costs more and takes longer. Air obeys the unseen-birth rule. Everything moves at its real speed, with no fixed timers. |
| D4 | Alarm answers follow the same principle; the director may answer an alarm with air insertion. |
| D5 | Any enemy soldiers standing in an empty town of the player's drain it. There is no ownership filter, and a passing column drains too. |
| D6 | A watched road means the force is born farther back, out of view, and the player watches it arrive. Keep the 250 m bound around the target. If the whole route is in view, start beyond the base (off-map for air, or the next base back). No turn-back as normal behaviour. |
| D7 | Waves: three big moments. A wave goes where the director judges best and CAN retake bases. The win latches only when every enemy military zone is held at once, and nothing changes after it latches. |
| D8 | A starved pool weakens the occupier visibly until a wave lands. A slow passive refill also lets a quiet player watch it push out again. |
| D9 | The director knows only what its men saw (sightings, alarms, heat, flips), never the player's true position. |
| D10 | The campaign is lost in the final case of D13. The end path mirrors END1 with a lose debriefing, and saves stay available. |
| D11 | A disguised player does not hold a town. A slung weapon is visible to an observer who sees his back. Drain presence is: exposed, or a weapon visible to an occupier observer. |
| D12 | Player messaging is deferred. Each director action writes one INFO log line, and a gm* command lists operations. No hints or journal lines from the director yet. (2026-10-14: the per-action INFO lines are dev-only, at INFO under `--dev` or `forcesLog = 1` and at DEBUG otherwise, 6.6; journal integration is GearUnclear/CWR#98.) |
| D13 | When the occupier takes the player's HQ (the Camp or a moved HQ town), the HQ is DESTROYED and the player must establish a new one through the existing HQ establish flow. Losing the HQ is not the end. The campaign is lost only when the player has no HQ and holds no base and no town. Respawn at the HQ is out of scope (its own issue). |
| D14 | Only a force the director deliberately sends against the HQ can take it. Passing patrols, columns and QRFs never drain or capture the HQ. |
| D15 | The passive refill takes an exhausted occupier back to full in about 4 hours of play, garrison regrowth included. The pool bands have hysteresis. |
| D16 | The occupier may go for player-held bases in ordinary operations whenever it is strong, not only in waves. |
| D17 | Jets and attack helicopters may strike towns the player holds as well as reported fighters; the civilian cost is accepted. They strike what was reported, or the town itself, never a hidden player position (D9). |
| D18 | A held town weakened by a passing enemy stays weakened. It recovers only while the player or resistance fighters are present. |
| D19 | Any enemy unit that clears the player's squad and stands in a held base takes it back through the mirrored capture meter. |
| D20 | Helicopters may land troops in plain view: born unseen, never within 250 m of the town. Old saves get the core behaviour, but keep the old QRF and heat cooling until a new campaign. |
| D21 | An air strike on a town costs DRAMATIC support for BOTH sides: people hate war. The occupier gets a support counterpart that a strike cuts hard. |
| D22 | Vehicles in the HQ garage are destroyed with the HQ. |
| D23 | A uniformed (non-disguised) player always holds the town he stands in, weapon out or not. Only a disguised player needs a visible weapon or exposure. #86 narrows capture only. |
| D24 | After destruction the player may establish a new HQ anywhere. The HQ is always losable: its capture is a meter tied to the HQ position (deliberate assault only, D14), independent of zone ownership. An HQ inside an enemy zone never suppresses that zone's defenders or census. |

### 0.3 The design in one paragraph

**The pool.** A finite theatre pool, counted in soldier-equivalents, pays for
everything the occupier does:
- regrowing garrisons;
- refilling towns it retakes;
- buying operations: a squad on foot or in trucks, armour, a helicopter
  insertion, a jet or attack-helicopter strike.

It refills slowly (full in about 4 hours from empty), and jumps when one of
three reinforcement waves lands.

**The director.** It wakes when the second distinct town is taken. It knows
only what its men saw, and spends a small command budget:
- on the hottest towns;
- when strong, on the player's bases and quiet towns;
- with a deliberate force, on his HQ.

For each operation it buys the cheapest package that is lethal enough, and
the fastest one when the matter is urgent.

**Movement.** Every force is born out of the player's perception, at its
base or farther back, never within 250 m of its target, and moves at its
real speed.

**Who retakes what.**
- Any enemy standing in an empty liberated town or base retakes it.
- Only a deliberate assault takes the HQ, through a meter at the HQ itself.
  The HQ is then destroyed with its cache and garage, and the player may set
  up a new one anywhere.

**Air strikes on towns** cut both sides' standing there sharply.

**Win and lose.** The campaign is won when every enemy base is held at once,
and lost when the player has no HQ, no base and no town.

## 1. Ground truth

Citations name functions; line numbers drift.

### 1.1 Counter-attack service (`CounterAttack.{hpp,cpp}`)

**Dispatch (`4400fe9`): `Decide`.**
- k is the larger of `_townsTaken` (every flip) and the non-exempt towns
  held before this event in the tick's batch: when several towns flip in
  one zone tick, `OnZoneEvents` excludes the later captures of the same
  batch, so the first sees k and the second k + 1 [code].
- k = 0 uses `counterAttackFirstTownChance` (0.4); k >= 1 uses
  `counterAttackChance` (1.0).
- `ForceSize = counterAttackSize (0.5) * 12 * max(1, 1 + counterAttackWarLevelStep (0.25) * (WL - 1))`,
  clamped to 1..12.
- Seams: `Decide`, `LaunchFrom` (immediate, debits a source, CITY targets
  only, one record per target, refuses exempt towns), `TownsTaken`.

**Spare.**
`min(stored + live - floor(garrisonMax * counterAttackSourceKeep 0.25), useLive ? live : stored)`.

**Unseen birth (`450327e`).**
- **Candidates.** At launch: the road-snapped base, then A* road route
  points toward the town (`RouteStep` 50 m). For an abstract force the
  player approached: points backward from its progress toward the base.
- **Gate.** The first point that Traffic's #53 gate (`PerceptionCamera` +
  `CanExposeSpawn`) rates hidden wins. Cone-only points count only within
  300 m of the base (`ConeOnlyBaseRadius`). Nothing is born within
  `counterAttackBirthMinToTarget` (250 m) of the town.
- **Nothing passes.** The force waits with its clock stopped and does
  nothing to the town, retrying every 30 s. After `counterAttackBirthGiveUp`
  (600 s) it turns back and refunds its base.
- **March.** Squads march SAFE (`MarchSpeed` 1.0 m/s allowance) and switch
  to AWARE at the town edge. They return to abstract travel when the squad
  and the town are out of the player's world.
- **Census.** The census counts a counter-attack group only in its own
  target town (flagged to be revisited; D5 revisits it).
- **Speed.** Abstract travel is `counterAttackSpeed` 4 m/s over
  `counterAttackRoadFactor` 1.3.

**Resolution.**
- Off-screen: `Fight` (`HoldOdds = D*w / (A + D*w)`, A = BODIES, w = 3) with
  fighters present, else `CAHolding` drains the town.
- `CATravel` turns back when its town reverts under it.
- Live squads: `counterAttackMaxLive` 2, `counterAttackLiveTimeout` 1200 s.
  A squad standing in its reverted town is adopted (`AdoptLive`).

### 1.2 Zones, garrisons, HQ

- **`ZoneRecord`.** It holds `garrison` (a FLOAT reserve), `garrisonMax`,
  `heat`, `flipDay` / `flipMinute`, `recoverClock` and `income`. Seeding
  (`seedCities = 1` on Abel and Sinai) skips the settlement probe.
- **Town gate.** Every non-held CITY keeps `cityGarrison` 4, and support
  cannot rise while any of them lives.
- **Retake branch** (`EvaluateTick`). Occupier presence with no resistance
  presence drains a held town by 5 per 3 s, and it reverts below 30
  (`RevertTown`, refilled to `garrisonMax` from nothing). Support rises ONLY
  with resistance presence; there is no passive recovery.
- **Remote tick.** It skips resistance-held non-CITY zones.
- **Military zones never revert** (decision R).
- **Military capture.** `defenders = occ > 0 || garrison >= 1`. A capture
  sets `income = defaultIncome` (25) when it was below 1.
- **`capture.sqs`.** On every `captured` event it spawns a 3-man hold squad
  and writes "Liberated".
- **Player presence** (`GatherInputs`). While `gmUndercover` is true, the
  player counts only per `PlayerCountsAsPresence` (exposed = the global
  `LivingWitnessCount() > 0`, weapon in hands, or warrior outfit); otherwise
  he always counts.
- **The undercover classifier** (`EvaluateUndercoverRule`) already rates a
  slung weapon per observer, with `undercoverBackArcCos`,
  `undercoverSlungBoost` and `undercoverNoticeRadius`.
- **`GarrisonCache`.**
  - `SpawnGarrison` places bodies at +-50 m of the centre, ungated.
  - **`DespawnGarrison` ASSIGNS `garrison = survivors`.**
  - The wiped-garrison release re-materializes a waiting reserve beside the
    player.
  - Partial-spawn losses are not refunded.
  - A flipped base's live garrison is written onto the resistance record.
  - D2 (`StepRecover`): 1 body per 600 s per calm zone, from nothing.
- **HQ** (`GuerrillaBase`):
  - one HQ per campaign, in any zone (`Establish`, the `gmHqEstablish`
    action in `market.sqs`, or the new-game START TOWN);
  - a persistent weapon cache (a keep-when-empty holder) and a garage ring
    whose locked vehicles are invulnerable;
  - re-establishing MOVES it (the cache contents travel);
  - `ZoneName()`, `IsEstablished()`.

  Recruiting is anchored at the first CAMP zone within 30 m
  (`recruit.sqs`), not at the HQ (#79 D5).

### 1.3 Other forces, factions, the end path

**Other forces.**
- **QRF** (`qrf.sqs`): one at a time on a RED zone, about 600 m out; stands
  down below RED; teleports when stalled; free.
- **Traffic and Legends** are free.

**Faction ladders.**
- WEST `{"Jeep", "Truck5t", "M113", "M60"}` and EAST rung 0 `UAZ`
  (`guerrilla-factions.hpp`).
- LoBo Hizballah `{Ural, JeepMG, Ural zsu23, JeepAT}` (`lobo-factions.hpp`).
- No faction lists aircraft.

**Engine facts for air** (read-only sweep):
- `EntityAI::MakeAirborne()` exists and is honoured only by the mission
  loader (`AICenterImpl.cpp`). SQF `createVehicle` cannot spawn airborne.
- Beyond the grid the world is sea (`YOutsideMap`).
- `AirplaneAuto` starts in `TaxiIn` on the ILS taxi path.
- Helicopter landing runs through `Transport::LandStarted`. `FindStopPosition`
  prefers the nearest `HeliH` within 500 m of the HELICOPTER.
- TR UNLOAD (`AIArcadeActions.inc`) ejects only cargo of a different group
  and refuses get-out above 5 m.
- Deleting a transport ejects its crew.
- Arcade waypoints did not move seated crews in the Traffic probes; only
  `IssueCommand` to the driver did.

**The end path.**
- **The win.** `CampaignEnd` latches `_won` when every roster zone is held
  at once. `RequestEnd` sets `EMEnd1` after `campaignEndDelay`.
- **The debriefing.** `UIMapDialogs.cpp` adds the Guerrilla debriefing text
  only for `EMEnd1` (`DebriefingText`, false unless won).
- **What follows.** `DisplayOutro` plays `OutroWin` for `EMEnd1..6`.
  `EMLoser` takes the same exit route (`exit.sqs`, `DestroyMap`, debriefing,
  `OutroLoose`, main menu). Retry/Load opens only for `EMKilled`.
- **`campaignEnd = 0`** skips the roster entirely.

## 2. Heat, alert and war level as built

**Alert.**
- **Input.** A zone's `knows` is the max `FadingSideAccuracy` of the PLAYER
  among occupier groups within 150 m (traffic crews 1.5 km).
- **Bands.** RED at >= 1.5, YELLOW at >= 0.5, every 5 s (`alertInterval`).
- **Position leak.** With no target position, `GatherInputs` falls back to
  the player's real position.

**Heat** (0..100 per zone).

| Event | Heat |
|---|---|
| Flip to the resistance | +40 |
| YELLOW / RED edge | +4 / +15 |
| Undercover compromise / later witness | +25 / +8 |
| Traffic wipe | +4 |

- **Decay.** -1 per 10 s on GREEN zones (`escalation.sqs`).
- **Readers.** Only shakedown odds and the journal.

**War level.** It is set from the share of all zones held (20/40/55/70/85 %
map to WL 2..6) and read by:
- tiers;
- the QRF escort rung;
- `ForceSize`;
- convoy chance and curfew;
- undercover detection `1 + 0.5 WL / 10`;
- the resistance hold squads and recruit cap.

## 3. Where a force may appear, and how it moves

### 3.1 Birth (invariant S, with D6 and D20)

Every force this design moves is born only at a point Traffic's #53 gate
rates imperceptible.

**The gate:**
- `PerceptionCamera` + `CanExposeSpawn` against `EffectiveBand`;
- the 300 m audible floor (`trafficMinSpawnDist`), even when hidden;
- no water for ground forces;
- cone-only points only within 300 m of a ground force's source base;
- for aircraft, the gate is evaluated **at the birth altitude**, because a
  ridge that hides a ground point need not hide an aircraft 100-150 m above
  it (`OccupierAir::ExposureAt` with `AirViewer`: Traffic's camera, band and
  terrain ray at the aircraft's height). The audible floor for aircraft is
  `airAudible` (800 m), not 300 m; a point beyond the effective band in 3D
  from the eye is hidden even inside the view cone;
- ground hulls are born and deleted under the **hull band**: at night
  (`NightEffect` above 0.2) Traffic's headlight band, with a 2.5 m
  (roof-high) terrain ray in place of the infantry 1.8 m (review M8).

**The ground chain**, in order:
1. **At launch.** The road-snapped source base, then route points toward
   the target, never within 250 m of it.
2. **On the road.** Backward from the force's progress point toward its
   base.
3. **The road route** (`RoadRoute`). Stage 1 is a multi-start road A*
   (starts within 300 m, widening to 900 m). Stage 2, the join, runs when
   the source's roads never reach the target's net: the route follows the
   source's pocket road, leaves it at the link with the best cost (road so
   far + 2 x the cross-country gap + the straight distance on to the
   target) and crosses country to a town-net link within 2000 m of the
   source (`RoadRouteServiceJoinRadius`), the best by that cost, not the
   nearest. The join lands only on the source's landmass (below): it never
   crosses the sea. The Southeast Compound on Abel was routed this way
   until `bcca80e`, but it stands on its own island (about 800 m of deep
   sea to Le Port; the old join leg crossed the sea, and its W and S gates
   are closed wire gates), so it is now an air-only source.
   **The landmass rule** (`bcca80e`). `RoadRoute::Landmass` labels the
   landscape's 50 m geography cells joined over land (8 neighbours; deep
   water parts them, a road over water is a bridge); `SameLandmass` is
   false only when both ends are known and differ (`gmLandmass`,
   `gmGeoGrid`). A ground force never comes from, and a Traffic vehicle
   never drives to, a base on another landmass than its target
   (`PickSource`, `Resource` with an INFO line, `BuildSources`, the wave
   fallback, Traffic's `PickDest`). Such a base is an AIR-ONLY source
   (`PackageSource::airOnly`): the director quotes only a helicopter
   insertion from it, and step 6 re-plans as air at once when no other
   base shares the target's landmass.
4. **Next base back, at once (D6).** If nothing between base A and the
   250 m bound passes, the operation is re-sourced immediately
   (`CounterAttack::Resource`) to the next other occupier base B on the
   target's landmass, nearest the target first, whose STORED spare covers
   the whole force and whose route yields a point passing the gate (the
   probe runs under the hull gate). A base route is the road A*, else the
   straight cross-country line (an accepted deviation); ground sources
   share the target's landmass, so that line stays on land, and a base on
   another landmass is never tried. A is refunded up to its room, the surplus
   (or a lost base's share) to the pool; B pays; the arrival clock restarts
   from B. Then each further base.
5. **Wait.** Only when no base passes: the force waits with its arrival
   clock stopped, doing nothing to the target, and retries its own route
   and the re-source every 30 s. After `counterAttackBirthGiveUp` (600 s)
   it tries the re-source once more.
6. **Air re-plan** (landed, `bcca80e`; `CounterAttack::AirReplan`). After
   the wait's last re-source fails, a FUNDED ground operation becomes a
   helicopter insertion when the catalogue offers the transport
   helicopter, its seats hold the bodies and the pool pays it
   (`OccupierForces::SpendOnOp`); it does so at once, without the wait,
   when no other base shares the target's landmass. The airframe flies
   from the occupier-held AIRFIELD nearest the target, else off-map; the
   ground hull's price goes back through the ledger, and the airframe goes
   on the operation's ledger row like a ground hull (the row's package
   becomes the transport helicopter, its outstanding grows by the price),
   so its refund or loss settles through the row. Only a re-plan no row
   can take is settled straight to the pool (`airReplanned`). A legacy
   record never re-plans.
7. **Cancel unseen.** Otherwise the operation is cancelled while still
   abstract:
   - its bodies return to their current source up to its room, the rest to
     the pool; the hull and escort prices are refunded;
   - the town gets the 30 min cooldown;
   - one INFO line.

   An unborn force vanishing off-map is not a visible turn-back, so D6
   holds.
8. **Edge hold.** A force whose progress point is already inside the
   bound while the player stands in (or the bubble covers) the target is
   held at the target edge, doing nothing. The hold counts toward
   `counterAttackBirthGiveUp`; then the force is born at the edge out of
   sight (`ArriveLive`, searching back from the last route point beyond
   the bound) or, with no such point, cancelled unseen. A suspended
   HOLDING force does the same (review M2). A born-at-edge attempt that
   must wait for a budget drops back into the hold for another full
   give-up.
9. **Every record, key on.** With `occupierForces` on, legacy (DORMANT)
   records take the same chain and never turn back (D6). With the key off
   an unfunded record waits and turns back after `counterAttackBirthGiveUp`,
   its strength returning to the base, and an edge hold retries every 30 s
   with no give-up.
10. **Slots.** An operation that has not been born yet does not count
    against `forcesOpsMax`.

**Air births** (the spike's recipe, `AIR-SPIKE.md`, `OccupierAir`):
- **Helicopters** are born **airborne**: `NewNonAIVehicle`, oriented and
  `SetTransform` first, then `SetFlyingHeight(altAgl)` (writes
  `_defPilotHeight`) BEFORE `MakeAirborne()`, which returns the height to
  add, then `SetTransform`, `Init`, `AddVehicle`. Crew and cargo are two
  groups, crew CARELESS / GREEN, cargo SAFE. The place is over the
  occupier's airfield, or **beyond the map edge** at the parent nation's
  side (`forcesOffMapEdge`, default the edge nearest the target; the point
  lies 1500 m beyond the map, level with the target). Landed in `bcca80e`:
  the origin is the occupier-held AIRFIELD nearest the target, else the
  edge, fixed at launch; the birth is the first point the altitude gate
  rates hidden, searched back from the progress point to the origin, then
  1500 to 5500 m beyond the edge, never within 250 m of the target.
- **Jets** are always born airborne over or beyond the airfield, never on
  the ground: `AirplaneAuto` would otherwise taxi the world's single ILS
  path.
- **Off-map is sea and it renders.** An off-map point is admissible only
  where the gate passes at altitude (above). Off-map is not automatically
  unseen.
- **Wiring** (landed, `bcca80e`). `CounterAttack` drives `OccupierAir`
  (`CounterAttackAir.cpp`; the contract in `CounterAttackOrders.hpp`:
  `CAKStrike`, `CAKAlarm`, `airOrigin`, `strikeZone` / `strikePos`,
  `alarmZone` / `alarmPos`, `CAOpAirborne` / `CAOpLanded` / `CAOpStruck` /
  `CAOpAlarmAnswered`). The record's air fields, the `AirHomes` list and
  `OccupierAir`'s records are saved in the CounterAttack block.

### 3.2 Movement at real speed (D3, D4)

There are no fixed timers. Abstract travel uses the package's real speed.
Config `maxSpeed` is km/h, converted to m/s:

| Package | Abstract speed | Route |
|---|---|---|
| Foot | 1.5 m/s | road route (or cross-country, `450327e`) |
| Ground vehicles | 0.6 x the slowest hull's `maxSpeed` | road route |
| Aircraft | 0.8 x the airframe's `maxSpeed` | straight line |

**These real speeds apply only with `occupierForces` on.** With it off,
`counterAttackSpeed` (4 m/s), `counterAttackRoadFactor` and `MarchSpeed`
keep today's pacing, so existing templates and tests keep their outcomes.

**Ground vehicles.**
- They are driven with the Traffic idiom: `Command::Move` through
  `IssueCommand` to the drivers.
- At the town edge (250 m) the carrier brakes and the infantry dismounts
  once the hull is at or below 1.5 m/s, or after 15 s of braking (so the
  dismount may happen inside the bound), and assaults. Armed hulls
  support; a truck holds back where it stopped.
- **Columns** (phase 5): a carrier (truck or APC) plus at most one escort
  (armed car, APC or tank). The escort is born `EscortGap` (20 m) behind
  the carrier's birth point under the same hull gate, and both drive the
  same legs: the carrier waits while the escort is more than
  `EscortSpread` (150 m) behind and drives on once it is inside
  `EscortRejoin` (60 m). From the carrier's first braking frame the escort
  drives up to the town edge and supports, crew AWARE and firing; an armed
  carrier supports too.
- On returning to abstract travel the hull is DELETED with no refund, and
  re-created on the next birth. The record holds its price. A seated man is
  moved out of the vehicle first, then deleted, crew before hull
  (`EngineOps`; `Traffic.cpp` keeps the old idiom, open).
- **Leftovers** (review M3, L7). A lost hull's crew group, an intact hull
  whose crew is dead or gone, and every wiped or dead squad group go onto
  a saved list (`Leftovers`), checked every 30 s: a hull is deleted when
  intact, unoccupied and `HullUnseen`; a group when all are dead or its
  leader is beyond 850 m and every body, living or dead, is unseen. Living
  stragglers go home as bodies up to the room. A wreck stays. Entries count
  against the 40-group budget until deleted.

**Helicopter insertion.**
- **LZ.** 300-500 m from the target, on the side away from reported
  fighters, chosen natively: object-free, dry, gradient within the engine's
  landing limit.
- **LZ probe** (as built). Candidates on the 300..500 m ring, the side away
  from a threat first; each snaps to the centre of its 50 m terrain cell,
  which must pass the engine's landing test (no water, not full, no
  objects, gradient below 4) plus no forest flag and a slope of at most
  0.25 across 22 m.
- **Landing.** An invisible `HeliHEmpty` pad (falling back to `HeliH`) is
  placed at the LZ with `GWorld->AddBuilding`, not `AddVehicle`, because
  `FindStopPosition` walks only the buildings list, within 500 m. The
  pilot is moved with a `Command::Move` through `IssueCommand`;
  `LandStarted(LMLand)` is ordered within `landOrderDist` (450 m) of the
  pad and re-issued every 5 s until down (a single order waved off).
  Touchdown is height at most 5 m and speed at most 3 m/s for 1 s.
- **Groups.** Crew and cargo are TWO groups. TR UNLOAD ejects only a foreign
  group's cargo; both count against the 40-group budget.
- **Unload.** The cargo gets `OrderGetIn(false)`, `AllowGetIn(false)`,
  `DoGetOut`, then a per-unit `AIUnit::UnassignVehicle` (never the
  group's, which clears the crew's seats; without it the helicopter landed
  again beside the squad), and walks on the target.
- **Lift-off.** With no cargo seated: `LandFinished()`, the pilot set to
  Wait when Stopping or Stopped (without it a 30 m hover over the LZ), then
  a fresh `Command::Move` home, re-issued every 45 s while it stays down.
  The pad stays until the hull is deleted (`bcca80e`; the spike removed it
  at departure). The unloaded cargo becomes the record's live squad and
  walks the straight line from the LZ.
- **Failure** (landed, `bcca80e`). A landing more than 100 m from the LZ
  (or a timeout, or no pilot) is a failed insertion: the airframe flies
  home with its cargo and is removed unseen; the record goes back to
  abstract flight once (the airframe re-created at the next birth, no
  refund), then is cancelled unseen. No LZ, or no airframe: the force goes
  on foot, its price refunded. A shot-down airframe is lost; whoever got
  out fights on.
- **D20.** The landing may be in plain view.
- **Removal.** The airframe returns to its origin and is deleted only where
  it is unseen at its altitude for 2 s, beyond the map edge included
  (rendered sea, not automatically out of view): the CREW FIRST (with any
  cargo still seated), the hull one step later, then the pad, because
  deleting a transport ejects its crew. Every crewman leaves his seat
  before he is deleted (the `EngineOps::LeaveVehicle` idiom), so none
  lingers in the world's out-vehicle list. **Unseen for an aircraft**
  (orchestrator decision, 2026-10-14, landed in `bcca80e`): out of the
  view cone and terrain-masked at altitude, or BEYOND the perception band
  in 3D even inside the cone (`OccupierAir::UnseenAt`); `departTimeout`
  (900 s) stays as the backstop. The price is then refunded
  (`CAOpHullRefund`) and the crew go home as bodies; a destroyed airframe
  is lost (`CAOpHullLost`).

**Jet or attack-helicopter strike** (D17):
- It flies to the target (AWARE / RED) and, within 1.25 x the orbit radius,
  searches in COMBAT / open fire on an 8-point orbit 900 m out, a leg every
  20 s, for `forcesSortieTime` (300 s, read from config since
  `bcca80e`), then flies home and is removed as above. The spike proved
  birth, search, reveal and removal, not damage on a target; attack
  helicopters share the jet's path. Landed in `bcca80e`: a strike
  (`CAKStrike`, no bodies) is abstract on its clock and live as an
  `OccupierAir` sortie only while its target is in the bubble (an accepted
  deviation); `CAOpStruck` goes out once, live or abstract, and the
  director applies the 7.3 effects on it.
- Its own sensors acquire targets.
- `AIGroup::AddTarget` (reveal) is used only for sightings younger than
  60 s, with accuracy < 1 (0.5 as built), re-issued every 10 s while in
  range, and only for resistance AI within 100 m of the sighting, never
  the player. The crew never gets an exact hidden position (D9).

### 3.3 No pop-in (S5)

A body may enter a zone within `cacheRadius` + 50 m of the player only:
- (i) as a live force that moved in; or
- (ii) by `GarrisonCache` materialization when every body's spawn point
  passes the gate's turn-proof tier. The points are
  `GarrisonCache::SpawnPoint` (hashed from the zone name and the body's
  index), the same ones `SpawnGarrison` places the bodies on; the release
  test checks them and the 3 x 3 grid of the placement square (L11).

**Every other reserve write there is a PENDING deposit**
(`GarrisonCache::Deposit(zone, n, origin)`):
- it does not defend and does not materialize;
- it moves into the reserve when the zone leaves the bubble or the points
  become turn-proof;
- if the owner changes it is refunded to the pool. With the key off (phase
  1, no pool) it goes back to its source zone if that zone is still
  occupier-held, and is otherwise discarded, as such bodies are today;
- it is saved on `ZoneSaveState`;
- it is flushed before `StepRecover`.

**Where the player stands.** A write outside the zone tick (a revert, a
wave's allocation) takes the player from the last zone tick. Before the
first tick since a load it asks the world for the player now, and with no
player known the write is pending (`ZoneRegistry::AddReserveNow`, L10).

**The writes it catches:** revert refills, returns to a base, revert surplus,
and the wiped-garrison remainder, which no longer appears beside the player
who just killed the last defender.

**Known limitation.** Ordinary materialization at 800 m stays ungated, so a
raised view distance can still show pop-in.

## 4. Overview

```
   passive refill (full in ~4 h, garrisons included) + waves (three)
                          |
                          v
   +------------------ POOL (soldier-equivalents) -------------------------+
   |            |                     |                         |          |
   v            v                     v                         v          v
 D2 regrowth  town / base refill   director operations       alarm      wave strikes
 (garrisons)  (pending, S5)        (towns, bases when strong, answers    (towns, bases,
                                    HQ assaults, push-out)               HQ in cluster)
   |                                  |                                    |
   +---- survivors, intact hulls, refunds, cancelled unborn ops -----------+
```

**Ownership.**
- `OccupierForces` (new) owns the pool, control, director, tokens, intel,
  waves and the package catalogue.
- `CounterAttack` moves forces. Its records gain `kind` (RETAKE, BASE,
  HQ_ASSAULT, ALARM, REINFORCE, STRIKE), `origin`, `package`, `funding` and
  hull refs.
- `CampaignEnd` owns win and lose.
- `GuerrillaBase` owns HQ destruction.

## 5. The pool

### 5.1 Unit and size

**Unit.** One unit = one soldier. Hulls are priced in the same units.

**Size.** At its first active tick the service takes its own roster
(`IsMilitaryType && IsAuthoredOccupier`, independent of `campaignEnd`) and
computes:
- **R**: the roster's summed `garrisonMax`.
- **K**: every administered, non-exempt CITY zone, where a SEEDED town
  (one the scaffold made from a map label, `seedCities`) counts only when
  it passes the settlement probe. An authored (template) CITY always
  counts.
  - **The probe** is the Auto seed's own `LandscapeSettlementProbe`: the
    label stands on dry land and at least 3 house-sized buildings
    (bounding sphere over 4 m) stand within 300 m of it (16). Water or
    fewer buildings make it a label, not a settlement.
  - **When.** `OccupierForces::ActivateProbed` runs it once, at the first
    active tick, on every seeded CITY whose zone row holds no verdict yet
    (`ZoneRegistry::ProbeSeededSettlements`), and stores the verdict on
    the row (`settlement`: 1 or -1, saved). It then sizes from the rows,
    so K is the same in every session. A save without the block probes
    when it derives (`DeriveFromExistingSave`). A block saved before the
    probe keeps its saved K; nothing re-sizes a running pool.
  - **Reporting.** One INFO line per town ("not counted (a seeded label,
    not a settlement)"), one listing every probed-out town, the probed-out
    count in the sizing line and in `gmOccupierForces` slot 29.
  - An old save's K counts by the AUTHORED owner, so towns the player
    already holds stay in K (review M7).
- **G**: the summed `garrisonMax` of the K towns.
- `poolStart = max(24, R + 2K)`.

An empty roster turns the service off.

| Template | R | K (measured) | poolStart | Wave (half, 12..48) |
|---|---|---|---|---|
| Abel | 32 | 14 (13 seeded towns probed, all settlements) | 60 | 30 |
| Sinai | 14 | 26 (37 before the probe; 11 seeded labels probed out: 6 in the water, 5 with 0 or 1 building) | 66 (88 before) | 33 (44 before) |
| Lebanon80 | 14 | 3 | 24 | 12 |
| Eden | 18 | 18 + Auto seeds | at least 54 | at least 27 |

### 5.2 Passive refill (D8, D15)

The refill is sized so an EXHAUSTED occupier (pool 0, every garrison it
still owns empty) is FULL again (pool at poolStart, every garrison at
`garrisonMax`) after `forcesPoolRefillHours` (4 h) of play:

`refill per hour = (poolStart + R + G) / 4`, accrued each director tick.

- **Who takes it.** D2 regrowth draws first, still at most 1 body per 600 s
  per zone. Whatever D2 does not take accumulates in the pool, up to
  poolStart.
- **Abel.** (48 + 44 + 56) / 4 = 37 per hour (poolStart 48 and R 44 since
  the generated bases, 5.7). An exhausted Abel refills its 100 garrison
  bodies first (the unit simulation measured 2.6 h for the 88 bodies before
  the generated bases; the naive figure is now 2.7 h), then its pool, so 4 h
  in all.
- **The per-zone cap binds only in the tail.** Abel's 18 zones can take up to
  108 an hour, so the cap is slack while many posts are short; in the last
  minutes only a few posts are still filling at 1 body per 600 s, the pool
  accrues meanwhile and reaches about 12 % just before they are full.
- **Waves** can push the pool above poolStart.
- **The quiet player's push-out** (6.4) starts once the pool is STRONG
  again, at about 3.5 h after full exhaustion, sooner from partial
  exhaustion.
- **4 h is the WORST case:** everything the occupier owns is empty. When
  exhaustion happens in play the player usually holds much of the island,
  so few occupier garrisons are short, and the pool alone refills 0 to 48
  at 37 an hour: about 1.3 h on Abel. The rate stays fixed from R + G on
  purpose; a beaten occupier rebuilds faster on what little it still holds.
- **Occupier standing** (7.3) scales the refill by
  `0.75 + 0.5 * meanStanding / 100`. At the default standing of 50 that is
  exactly 1, so the figures above hold. A heavily bombed theatre rebuilds
  more slowly.
- **The refill repays losses** (2026-10-16, 5.6). Every unit the refill
  credits to the pool repays the oldest unrepaid loss of the band's loss
  window first. This is what the band reads: a loss counts against the
  band until the refill has replaced it or it leaves the window.
- **Stored spare hulls come back first** (2026-10-16, 5.5). Each tick,
  before the refill, a stored spare hull rejoins the pool as far as the
  pool has room under the ceiling, and the wave surge's decay (7.1)
  returns its bodies, so the refill only fills what is still missing.

### 5.3 Spending

| Spend | Rule |
|---|---|
| D2 regrowth | `Draw(gained)`. An empty pool grants nothing and the clock is clamped. |
| Town or base refill on revert | `SettleRevert`, through `Deposit` (5.4) |
| Operations | The package price at launch. Infantry sourced from a base's spare is paid by that base (D2 refills it); hulls and crews are paid by the pool. |
| Alarm answers | As operations (8.2) |

An airframe's price leaves the pool at launch like any hull, but the band
counts it as committed until the airframe is home or lost (5.6). The pool's
own arithmetic, and so what the next operation can afford, is unchanged.

### 5.4 Who retakes what, and who garrisons it

| Zone | Retaken by | New garrison |
|---|---|---|
| Held town (not the HQ) | ANY occupier presence with no resistance presence: the drain (D5) | Bodies of a CounterAttack record HOLDING or LIVE at that town for that target become its garrison. Everything else (a passer-by, a column bound elsewhere, a Legend guard, the QRF) keeps its job, and the town's garrison is a pool-funded pending deposit (S5). |
| Held base (not the HQ) | ANY occupier presence that has cleared the resistance presence, through the mirrored capture meter (D19, 7.3) | The capturing record's bodies, if any; otherwise a pool-funded deposit, as for towns |
| The HQ itself (an object at `HqPos`, not a zone, D24) | ONLY an HQ_ASSAULT record, through the HQ meter (9.1); passers-by never touch it (D14) | None. The HQ is destroyed, and the zone around it follows its own ordinary rules. |

**A travelling record whose town reverts under it** (passer-by) does not
turn back with forces on. It continues as a top-up column (REINFORCE) when
the town's garrison is short, else returns.

**Support after a passer-by (D18).** It stays where the passer-by left it,
and rises only with resistance presence, as today. A town on a busy road
erodes until the player or his fighters come back (section 19).

### 5.5 Conservation

**Unconditional fixes** (phase 1, key on or off):
- **U1.** Additive despawn write-back.
- **U2.** Adopting a group keeps the zone's reserve.
- **U3.** Integral reserve writes.
- **U4.** Military capture counts the zone's own live garrison bodies within
  800 m of the base (needs a per-distance count in `GatherInputs`).
- **U5.** S5.

**Pool routing** (key on):

| Path | Fix |
|---|---|
| Holding force at revert | settle: its bodies first, up to the room; the surplus to the pool; the rest of the room a pool-funded `Deposit` |
| `Fight` win (key off only; with forces on every fight against present defenders is live, 6.5) | winners lose `min(A_bodies - 1, ceil(A_bodies * holdOdds * repelLoss))`; the rest settle |
| Adopted live squad | settled once in `RevertTown` |
| No base to return to | pool |
| Surplus over a base's room | pool |
| Flipped base's live garrison | pool |
| Partial-spawn failures (garrisons, operations, crews) | pool |
| Re-sourced operation | the old base refunded up to its room, the surplus (or a lost base's share) to the pool; the new base debited |
| Cancelled unborn operation | bodies to the current source up to its room, the rest to the pool; hull and escort prices refunded |
| Abstract REINFORCE arrival | into the target up to its room (`garrisonMax` - live - reserve - pending; pending inside the deposit bubble); the remainder home through the room-capped return, surplus to the pool |
| Live top-up squad (REINFORCE or alarm answer) at its town | up to its room (`garrisonMax` - live - reserve - pending): the stored reserve, then the pending deposit, give way to it (to the pool); the surplus splits off and walks home as Leftover stragglers (removed unseen, home up to the room, the rest to the pool) |
| Leftover stragglers (3.2) | home as bodies up to the room; a shortfall booked "left behind" |
| Pending deposit, owner changed | key on: the pool (`GarrisonCache::SetPendingRefundHook`); key off: back to the origin up to its room, else discarded |
| HQ_ASSAULT on `hqLost` | an abstract record's bodies to the pool; a live squad leaves unseen and comes home. The settled line says where they went (2026-10-16, video 4, which logged "@HQ retaken: 6 stay as its garrison" after refunding them): "[op N] HQ assault done: 1 back to the pool, 5 banked at Airfield" (the ceiling below; ", D dropped (every base full)" when no base had room), or "nobody of the assault left standing"; never the retake line |
| Surviving hull | price refunded when deleted at home unseen; each hull of a column apart. Over the ceiling, with `forcesBankHulls` 0 (the default, decision 9), the excess is a STORED SPARE HULL (below), never garrison bodies |
| Destroyed hull | lost |
| Hull deleted by a return to abstract travel | no refund; re-created on the next birth |

**The ceiling (2026-10-16).** Nothing but a wave puts the pool above
poolStart. Every row above that credits the pool goes through
`OccupierForces::Refund`, which caps the credit at `PoolCeiling()`
(poolStart plus the wave headroom). Without the cap the refill topped the
pool back up while an operation was out, and the operation's refund then
landed above poolStart (the comparison video: 51 of 50 after a jet opener,
53 of 50 after an APC came home).
- **The excess is banked** (owner decision, 2026-10-16: "bank surplus
  survivors into garrisons instead of dropping them"). What the ceiling
  keeps out of the pool becomes garrison at an occupier-held military base
  with room below its `garrisonMax` (room = `garrisonMax` - live -
  reserve - pending, as every room in this section). The base: the credit's source
  base when the caller names it and it has room; else the base nearest the
  credit's position (the source base's own position when only the source
  is named); else the base with the most room (ties: the zone order). A
  base takes what fits and the next one by the same rule takes the rest.
  The bodies go in through the S5 pending deposit (`GarrisonCache::Deposit`,
  3.3), so they join the reserve unseen or once the player has left, never
  beside him, and the director can spend them again from the reserve. One
  INFO line per base, `ledger: N unit(s) over poolStart banked at <base>`;
  `Banked()` counts them for the session (the bodies are saved on the zone
  rows, so nothing new is saved). Only bodies bank (survivors, crews,
  assault bodies): a hull home over the ceiling is stored (the next item)
  unless `forcesBankHulls` is 1, which restores the df9457e rule that
  banked a hull's units as bodies.
- **Stored spare hulls** (decision 9, 2026-10-16; video 5's "22 banked", an
  APC home to a full pool that became 22 bodies of garrison). With
  `forcesBankHulls` 0 the hull units a `CAOpHullRefund` brings over the
  ceiling (`RefundHull`) are kept as one spare hull row at the base the
  hull returns to: an airframe's airfield (`airOrigin`), else the
  operation's source. When that base is not an occupier-held military base
  or already stores `StoredHullsPerBase` (2) spares, the nearest occupier
  base with a garage (a military base that is not a minor post) takes it.
  One INFO line `ledger: APC over poolStart stored as a spare hull at
  <base> (N unit(s); ...)`. Each tick, before the refill, a spare rejoins
  the pool as far as the pool has room under the ceiling (`the spare APC
  stored at <base> rejoins the pool`), whole or in part. A spare at a base
  the occupier loses is lost with it (booked in `lost`, so it reads in the
  band's loss window). Only when no base can store one are the hull units
  dropped (`ledger: N hull unit(s) over poolStart dropped (APC: no occupier
  base can store a spare hull; ...)`). The rows are saved
  (`StoredHulls`); `gmOccupierLedger` lists them. Hull refunds that
  `CounterAttack` credits straight to the pool (the air re-plan's direct
  settlement) still go through `Refund` and bank as bodies until their call
  sites pass the hull.
- **The wave surge** (decision 5, 2026-10-16; 7.1). While a landed wave is
  on, what the ceiling keeps out of the pool is a surge above
  `garrisonMax` at the bases nearest the wave's cluster, not a bank or a
  drop: video 5's 9 dropped and 1 banked become 0 dropped. 7.1 has the
  rule; the decayed bodies that the pool cannot take stand down
  (`stoodDown`, saved).
- **Dropped, only when every base is full.** Only what no occupier base has
  room for is dropped, with one INFO line `ledger: N unit(s) over poolStart
  dropped (...; every occupier base at garrisonMax; ...)` per refund and
  the count kept in `Dropped()` (saved).
- **Sources named today.** The director's own refunds name the source
  (`LaunchAction`'s refusals, the hull refunds and cancelled-unborn refunds
  of `NoteOpEvent` and the test-lever rows), and the S5 refund hook names
  the deposit's origin and the lost zone's position. `CounterAttack`'s
  survivor, surplus, crew and HQ-assault refunds and `GarrisonCache`'s
  flipped-base refund do not yet, so they bank by the most room until their
  call sites pass the record's source (a follow-up in those files).
- **Wave headroom.** A landed wave lifts the pool above poolStart, as 7.1
  says, and the headroom rises to that overshoot, so wave strength spent
  and refunded comes back above poolStart. At each refund the headroom
  shrinks to at most what can still come back as wave strength: the
  pool's own overshoot, the units the ledger rows still hold and the
  refund itself. Wave strength lost in the field takes its headroom with
  it.
- **The refill** still fills only up to poolStart.
- **A forced pool** (`gmForcesForce ["pool", n]`) carries its own headroom
  (n - poolStart, or none).

**The equation.** With nothing live:

`pool + stored hulls + reserves + pending + op strength = poolStart + R + G + refill + waves - losses - dropped - stood down`

`dropped` is the units the ceiling dropped (`Dropped()`), only when no base
had room (and, for a hull, no base could store it); `losses` (the `lost`
field) stays the bodies killed, the hulls destroyed and the spares lost
with their base; `stood down` is the decayed surge the pool had no room
for (`StoodDown()`). The banked excess and the surge need no term of their
own: they move from op strength into `pending` (and later `reserves`).
Stored spare hulls are pool units in hull form, so they sit beside the
pool. The unit tests assert it, banked, surged, stored, dropped and stood
down.

### 5.6 Bands with hysteresis (D8, D15)

| Band | Enter | Leave | Effect |
|---|---|---|---|
| STRONG | strength > 66 % | strength < 60 % | One token per 10 min (`forcesTokenInterval[]`, 2026-10-15); base operations (D16) and push-out on towns AND bases allowed |
| STRETCHED | between | | One token per 17.5 min |
| THIN | strength < 33 % | strength > 40 % | One token per 30 min; no operation below 20 % (`forcesPoolFloor`; since 2026-10-17 the same floor also bounds one launch's own debit in every band, 6.3 "The reserve floor"); alarm answers only with the cheapest package; garrisons stop chasing YELLOW contacts (`qrf.sqs`) |
| SPENT | strength = 0 | strength > 10 % | No tokens; reverts left hollow; no alarm answers. D2 KEEPS RUNNING on the refill: garrisons first, as D15 says. The pool stays near 0 while garrisons are short, and the band leaves SPENT once, when the pool passes 10 % (measured: moments before the last short posts are full, because the per-zone D2 cap binds in the tail), so it does not flap. A pool that genuinely drains to 0 again re-enters SPENT; that is a real drain, not flapping. |

All shares are of poolStart.

**The band's input** (2026-10-16: decision 6 and the feel assessment's item
10; the levers audit found the band STRONG 99 % of video 5 because foot
losses were repaid by regrowth under the 37 an hour refill, and every dip
came from an airframe being out). The band is stepped, with the hysteresis
above, on the STRENGTH

`strength = min(pool + committed, max(poolStart, pool) - (lost - repaid))`

- `committed` (`CommittedAir`): the pool units the ledger rows of the
  director's (and the test levers') airframes still hold, a transport
  helicopter, an attack helicopter or a jet, from the launch debit to the
  airframe's `CAOpHullRefund` or `CAOpHullLost` (its crew until they
  settle). An airframe out is committed, not spent: video 5's two Mi17s
  took the pool from 52 to 31 and the band to STRETCHED for 100 s; the
  strength stays 52 and the band STRONG. A shot-down airframe leaves
  `committed` and enters `lost`: it demotes as a real loss. A ground hull
  out is not committed (a column still moves the band while it is out).
- `lost` and `repaid` (`LossesInWindow`, `RepaidInWindow`): the units booked
  lost (every `NoteLoss`: bodies killed in operations and garrisons, crews,
  hulls destroyed, spares lost with their base) in the last
  `lossWindowHours` (1.0 h, a constant with the gmForcesTune lever
  "lossWindowHours"), and the part of them the refill has repaid since
  (5.2: refill credits repay the oldest loss first). What the refill has
  not replaced counts against the band; a loss older than the window no
  longer does.
- `max(poolStart, pool)`: a wave's overshoot is strength on top.
- The pool arithmetic is unchanged: `Spend` still needs the units in the
  pool, and a director strike's keep-STRONG rule (7.3) still reads the
  pool after its price, so strikes stay as rare as before.

**When the band is stepped** (round 5, video 6). Once per transaction,
never between the bookings of one. Video 6 logged "band STRONG ->
STRETCHED (pool 23 of 48 ...)" and "STRETCHED -> STRONG (pool 34 of 48
...)" in the same second at 9982 s while the sampled pool read 34 before
and after: the repel's containment surge (a pool-funded deposit of 12) was
booked, the band stepped, then the column's refunds came home. The same
shape at 6414 s (a base revert's deposit for the new garrison, then the
adopted squad's refund), and at 8348 s a refill credit made the band
STRONG for zero time and admitted a STRONG-only push-out (op 14) whose
debit took it back. The rule now: `OccupierForces::Simulate` first closes
the previous frame's hold and steps the band ONCE on the strength
everything booked since left (ZoneRegistry's reverts, CounterAttack's and
OccupierAir's events, script levers), one line at most, then holds the
band for the new frame (`StepBandFrame`). Every gate in the frame reads
that one band: the director's whole tick (its refill, its candidates, its
tokens, `OpsMax`, push-out, the alarm answers), a deposit's SPENT test,
`CanOperate`. The tick's own refill and launches are stepped at the next
frame, so a refill that lifts the pool over 66 % opens STRONG for the next
director tick, not for the launch in the same tick. Two movements step
through the frame hold at once (`SettleBand`): a wave landing (its
allocation spends the landed bodies in the same tick, and a SPENT band
would refuse the hub top-up and every strike) and a forced pool (a lever
is a transaction by itself). An explicit `BandHold` (the launch and its
opener, the air re-plan, 197a7d3 and 3c9947b) still defers even those;
they nest inside the frame hold. Why a frame and not one step per director
tick: both keep intermediate ledger states out of the log and the gates,
but a step per tick would leave a genuine movement booked between ticks
(a repel, a column home, a base revert) unread by the alarm answers and
the deposits for up to 60 s, while every
booking of one transaction completes inside one service's frame call, so
the frame is the smallest boundary that is never inside one. It matches
5.6 and D15: the hysteresis holds the band against noise, and a band that
can be entered for zero time between two bookings of one movement is noise
the hysteresis never sees. Without `Simulate` (a bare ledger, the unit
tests) no frame hold is open and every seam steps at once, as before.

**On Malden** (Guerrilla.Abel: poolStart 48, the authored roster's 32 plus
2 x 14 towns less the 12 of the two generated outposts, conservation, 5.7;
R reads 44 with them, K 14, G 56; refill (48 + 44 + 56) / 4 = 37 an hour).
STRONG is left when the strength falls under 60 % (28.8: at 28), so 20
unrepaid losses inside an hour demote a full pool; THIN starts under 33 %
(15.84: at 15), so 33. A cell that kills 37 occupiers inside an hour while
D2 regrowth repays 12 of them (two source bases at 1 body per 600 s) leaves
25 unrepaid: strength 23 of 48, STRETCHED, while the pool itself reads 46
to 48 (STRONG by its own number). The player sees it as the director
slowing to one operation every 17.5 minutes, no push-outs, no base
operations and no strikes until the refill has repaid enough to lift the
strength over 66 % (31.68: 32, at most 16 unrepaid) or the losses leave the
hour. Before this rule the same hour changed nothing a player could see.
`gmOccupierLedger` reports the strength, `committed`, `lost` and `repaid`;
the band line names them (`band STRONG -> STRETCHED (pool 48 of 48;
strength 23: 0 committed in the air, 37 lost and 12 repaid in the last 1.0
h)`). The arithmetic is checked against the code's `PctBelow` /
`PctAbove` comparisons, not measured on a run.

### 5.7 Generated bases

Step 1 of the generated occupier bases (owner decisions of 2026-10-16; zones
only, no props: the layouts are step 2). A template opts in with
`generateBases = 1` in `CfgGuerrillaZones` (engine default 0); Abel and Eden
do, Sinai waits for its `playRegion[]` (#75), Lebanon80 stays off by design.
The pure core is `BaseGenerate.{hpp,cpp}` (`GenerateBases`, unit-tested on
synthetic cell grids); the seeding entry is `ZoneRegistry::GenerateBaseZones`,
run inside `LoadFromParams` after `SeedCityZones`, so it sees every town and
the 300 m city dedup never deletes a town a base stands near.

**The density rule** (binding). K is the CITY zones (a seeded label with no
settlement under it does not count when `seedCities = 1` seeds every label).
- Coverage: every town has a ground base on its own landmass within 2.5 km,
  straight line.
- Redundancy: every landmass that carries towns has at least three ground
  bases.
- Total: the occupier's military zones number between ceil(K / 3) and
  ceil(K / 2), never more than 10. Generation only adds, until coverage,
  redundancy and the floor hold; the ceiling (and the 64-zone cap) stops it.
- Spacing: 1.6 km from every occupier base, the Camp, the HQ (the START TOWN
  pick, `gmSelStartTown`) and the player's start; 300 m from any zone; 500 m
  from any town.
- Road: the centre 60 to 250 m from a road that joins the island network (a
  road component with at least a quarter of the largest one's links), never
  on a road; a third of the generated bases (rounded up) on a track 400 m or
  more from the trunk road (a dirt-track model, `RoadModelIsTrack`).
- Bearing: each town's two nearest sources 60 degrees apart as seen from the
  town. A scoring term (100 per town fixed or broken, capped at 200 either
  way); it shapes the choice and never adds a base.
- Terrain: per 50 m geography cell over the 150 m (3 x 3) footprint: no water,
  no forest, no `full` cell, slope band under 2 (below 0.10 rise over run);
  at the centre no road, no hard object, at most one object, 2 m above sea
  level, and the spawn square's corners (+-25 m) 0.5 m up; inside the play
  region and 200 m clear of the map edge.

**The choice.** Greedy, one base at a time, against the most urgent open need
(coverage, then redundancy, then the floor): 1000 per town the site newly
covers, plus the bearing term, plus up to 300 of quality (flat ground, the
middle of the road band, spread from the other bases, a seeded jitter of up to
50), plus 600 for a detected outpost and 600 for a track site while the track
share is short. The bonuses stay below one town, so they never cost an extra
base. The #76 detector's OUTPOST candidates (real props on the map) are the
preferred sites, which reverses the #76 rule that they are never seeded; they
pass the same rules except the terrain-cell tests, which their props would
fail. A generated base is an OUTPOST (garrison and reserve 6, spare 5,
enough for a town retake) wherever an outpost's footprint fits; since step 3
a minor kind (CHECKPOINT or COASTAL, garrison 5, spare 4) takes a need no
outpost site can serve (below), and the HELIPAD (6) is catalogued but never
placed. Names follow
"<nearest town within 2.5 km> <noun>", else "Hill <elevation> <noun>", made
unique with a number; a detected outpost keeps its detector name. The marker
is `gmZoneGen_<n>`; the zone is revealed by proximity like an authored one.

**Stored, never re-derived.** The picks come from one seed (the
`generateBasesSeed` key, else one `GRandGen` draw) at new game and are saved in
the zone block (`generatedBasesVersion` 1 and the `GeneratedBases` rows: name,
type, position, garrisonMax, reserve, countsForWin, heading, detected, track,
the step 2 layout seed). A load re-injects the stored rows before the dynamic
zone rows overlay them, whatever the key or the scorer say now; a save without
the marker predates the feature and generates nothing (one INFO line). Within
one session the decision is also kept across every `LoadFromParams` until
`Clear`.

**What works with no further code.** A generated base is a zone row like a
seeded base: `ownerConfig` OCCUPIER, the `seeded` flag, plus `generated` and
`countsForWin`. GarrisonCache spawns its garrison on hidden points, the
director sources from it (`CounterAttack::Spare`), a wave may hub at it as
the strongest base, capture and `occCapture` work by type and owner, Traffic
counts it as an endpoint. `gmGeneratedBases` lists the rows.

**The win roster.** `CampaignEnd::TakeRoster` takes every authored occupier
military zone and every generated one whose `countsForWin` holds and whose type
is not a minor kind (`IsMinorPostType`: CHECKPOINT, COASTAL, HELIPAD, RADAR).
The minor posts capture, source and alarm like any base but are left out of
the win (owner decision).

**Conservation** (the owner's rule, 2026-10-16: presence, not strength).
Generated garrisons are subtracted from poolStart. `OccupierForces::Size`
counts every generated occupier base (`CampaignEnd::IsGeneratedOccupier`) in
the pool's roster and in R, and books their summed garrisonMax as `Rgen`
(saved in the block, absent = 0): `poolStart = max(0, max(forcesPoolMin,
(R - Rgen) + forcesPoolPerTown x K) - Rgen)`. Abel: authored R 32, K 14,
Rgen 12 (two OUTPOSTs of 6), so poolStart 60 becomes 48 while R reads 44. The
refill, `(poolStart + R + G) / forcesPoolRefillHours`, is unchanged (37 an
hour on Abel), and a wave stays a share of `poolStart + Rgen` (Abel: 30), so
a generated base costs the occupier no wave strength. The generated bases
now count in control M, the final-stand count and the `gmOccupierForces`
roster. `CampaignEnd::IsAuthoredOccupier` still answers false for a
`generated` zone; the pool reads the generated predicate itself.

**Step 1 on the shipped templates** (2026-10-16, random seeds). Abel: two
OUTPOSTs, La Riviere Outpost about [3975, 4775] (south-west, covers six towns)
and Vigny Outpost about [4175, 7025] (west, on a track); every town within
2.5 km of a base afterwards, six military zones, the win roster six. Eden:
two, Le Moule Outpost about [3525, 4925] and Tyrone Outpost about [5425, 9375];
Regina stayed open under step 1 (no dry, flat, object-free 150 m square in
reach: the "Regina is open" INFO line counts the refusals; step 3 closes it
with a minor kind, below), the detected Levie Post
stands within 60 m of its road, and neither pick is on a track (the share is
reported short). Tests: `test_base_generate.cpp` (`[basegenerate]`),
`guerrilla_bases_generate_{abel,eden}`, `guerrilla_bases_generate_save.seq`.

**Step 2: the outpost layout.** Every generated OUTPOST on empty ground gets
one layout of stock statics, read from `CfgGuerrillaBaseLayouts >> Outpost` in
`guerrilla-mode/config/guerrilla-hq.hpp` (installed to `BIN\` with the HQ
classes): rows of `{class, dx, dz, dir[, jitter]}` in base-local metres, +dz
towards the access road (the stored heading), rotated by the heading. The
shipped Outpost has 12 rows: two `Fortress1` bunkers at the gate on the road
side, two `ViewTower` (the stock hlaska, scope 1) on opposite corners, four
`Wire` on the flanks, two tents (`CampEast|Camp`, `Camp`) and a `FlagCarrier`
at the rear (each 5 s pass pushes the base owner's flag texture), and a
`FuelStationNew2` fuel point on the road side as the one asymmetry. The
service is `BaseProps.{hpp,cpp}`, shaped like TownFlags: `InitMission` plans
and creates the props behind the loading screen at new game, before any
garrison spawns (2.6 ms for 19 props on Abel), keeps `LLink` rows, and
re-creates a prop only when its link comes back null (a deleted prop comes
back once, at its planned spot, within the 5 s pass). The props are world
buildings, so a save carries them in `_buildings` and the `GuerrillaBaseProps`
block carries only the rows and the planned base names: a load re-binds the
refs and never plans or creates a second set; a save from before step 2 gets
its layouts on the first tick. Rules: a 25 m yard at the centre stays clear (a
row whose footprint reaches into it is refused); nothing within 6 m of a road
link, measured from the footprint (`GRoadNet::IsOnRoad` with the footprint
radius plus 6 m); dry ground and no landscape object in the footprint; a
refused spot tries its left-right mirror, then both pushed 4 and 8 m outward,
then the row is dropped with an INFO line; class alternates (`A|B`) take the
first class the package can spawn (the plan-15 `Spawnable` probe) and none
skips the row with a WARN; fewer than `minProps` (8) placed warns, never
fails. A detected outpost (real map props) gets no layout. Statics stand
upright, seated by `PlaceOnSurface` with the buried-origin net; Fortress1 and
ViewTower sit 1.0 to 1.1 m dug in by their authoring, the rest at ground level.
Each created prop drops the cached oper fields of its 3 x 3 cells (the
`Object::SetDestroyed` idiom, Dammage.cpp), so pathing sees it.
`GarrisonCache` pushes a generated base's garrison spawn point out of every
prop's box plus 1.5 m before `CreateUnit` (its `FindFreePosition` only clears a
body out of the geometry), and `OccupierAir`'s LZ probe counts any cell centre
within 60 m of a prop as an object, since the probe reads geography fixed at
map load. Abel ground truth (2026-10-16): La Riviere Outpost places 10 of 12
rows and Vigny Outpost 9 (both gate bunkers there stand on landscape
objects), every prop within 60 m of its centre. Save size: the start-of-game
save of `guerrilla_bases_generate_save.seq` grows from 185,944 to 190,835
bytes (+4,891, about 257 bytes a prop). Frame cost: one pass of about twenty
link checks every 5 s. `gmBaseProps`, `gmBasePropsInside` and `gmBasePropsLz`
are the script surface. Tests: `test_base_props.cpp` (`[baseprops]`),
`guerrilla_bases_props_abel`, `guerrilla_bases_props_save.seq`.

**Step 3: the kinds, object-free footprints, Regina** (2026-10-17).
- *Object-free footprints.* The geography bits test objects at the centre
  cell only and skip light objects, so the step 2 picks stood among trees:
  Vigny Outpost lost both gate bunkers and its fuel point to them, La
  Riviere Outpost stood in an orchard (58 trees in its 3 x 3). The engine
  probe now hands the scorer every map object from the landscape object
  grid (`BaseSiteObjectClass`: a solid is a building, a wall, a fence or a
  gate, a rock; a tree is a tree, a bush, a forest piece; signs and stones
  under 0.75 m do not count), and a site whose layout reach (60 m) holds a
  solid costs 900, each tree 30, at most 900 in all: below one town, so a
  clear site wins among the sites serving the same need and clutter never
  costs a base. A detected outpost's own props are not clutter. The props
  keep a 4 m lane from every solid map object (the design's lanes; the
  layout key `lane`), so a prop never closes a gap a soldier or a car walks
  through; trees only need to clear the footprint. Abel ground truth: La
  Riviere Outpost moves to about [3775, 4675] (3 trees within 60 m, 11 of 12
  rows placed) and Vigny Outpost to about [4225, 7075] (5 trees, all 12 rows,
  the gate kept and walked through by a soldier, `guerrilla_bases_props_abel`
  p7 and p8); neither has a solid within 60 m.
- *The kinds.* Each kind has its own layout in `CfgGuerrillaBaseLayouts`:
  Outpost (12 rows), Checkpoint (9: a hut and the MG nest on the road side,
  the tower, tent and flag behind, yard 15 m), Coastal (10: tower, two
  bunkers, a sandbag line, hut, tent, flag, yard 20 m) and Helipad (8: two
  `HeliH` pads 44 m apart, the fuel point, sandbag rings, tower). A row
  alternate `$key` names the occupier descriptor's key, the faction style:
  `lobo-factions.hpp` gives the IDF, the Egyptian army and Syria LoBo's brown
  Baracken pieces (`basePropTower = "Brown_Turm"`, `basePropHut =
  "Brown_Wachhaus"`); a descriptor without the key, or a package without the
  class, falls through to the stock class after the bar.
- *The minor kinds placed.* The density rule is unchanged and decides how
  many bases and where; the kind follows the site. Within each need
  (coverage, redundancy, the floor) the outpost sites are tried first; only
  when none serves the need does a site whose centre 50 m cell passes the
  forest, slope and object rules take it, as a COASTAL post when deep water
  lies within 400 m, else a CHECKPOINT (its centre 60 to 100 m from the
  road, so its road-side rows stand on the shoulder). The water rule and the
  spawn square's height test still cover the whole 3 x 3 (the garrison
  spawns over +-50 m). Minor posts count as ground sources for coverage,
  redundancy, the floor and the ceiling, spare 4 for a town retake, and stay
  out of the win roster (`countsForWin` false and `IsMinorPostType`, the rule
  of step 1, unchanged). The HELIPAD is never generated: its role, the air
  origin when no airfield is held, needs the `HeldAirfield` extension first.
  `generateBasesMinor = 0` in `CfgGuerrillaZones` turns the minor kinds off.
- *Regina* (Eden). Logged with the minor kinds off: of the cells within
  2.5 km of Regina, 3,819 are refused for water (the coast), 1,666 for
  objects, 2,261 for slope, and 231 cells are one rule short of a site, every
  one of them the outpost's slope rule (the nearest 510 m from the town, at
  [7375, 2825]): the hills round Regina have flat 50 m cells but no flat
  150 m square. So the fix is the minor kinds' footprint, not a wider ring or
  an exception: a minor post's slope rule covers its own 50 m footprint.
  Ground truth: Morton Coastal Post about [5625, 3775] (deep water within
  400 m, 9 of 10 rows) closes Regina, and every Eden town is within 2.5 km of
  a ground base; Eden now generates three bases (Rgen 17), the win roster
  leaves the coastal post out.
- *Diagnostics.* For a town still open the log names its refusals and its
  near misses ("<town> near misses (one rule short of a site): slope 231
  (nearest [x, z], d m)"); each generated base's line names the solid and
  tree objects within 60 m. `gmBaseGenNotes` returns the notes,
  `gmBaseSiteObjects [x, z, r]` the map objects the scorer counts. Tests:
  `test_base_generate.cpp` (the kind catalogue, the clutter preference, the
  minor fallback and the near-miss notes), `test_base_props.cpp` (every
  kind's shipped layout, `$key`, the lane), `guerrilla_bases_generate_abel`
  g8, `guerrilla_bases_generate_eden` e2, e3, e6 to e8,
  `guerrilla_bases_props_abel` p7 and p8.

## 6. The director

### 6.1 Intel (D9)

**Sightings** are sampled at the alert cadence (5 s), in a NEW scan beside
`AlertMachine::GatherInputs`. For every occupier group it records the
resistance units the group holds as known targets (`Target::IsKnown`), with
reported positions and a time stamp. A destroyed group's last report is
kept ("it radioed before it died").

From the sightings the director builds:
- `knownFighters(zone)`: the latest count;
- `lastKnown(zone)`: the reported positions.

Both decay after 30 min.

**Inputs the director may use:** flips and owner changes, heat, alert
states, sightings, and its own forces' reports.

**Never used:**
- the true census (`guerCount`), with one exception: an informant tip
  (7.3). Each director tick a CITY with standing >= 50 and resistance HOLD
  presence rolls (hashed, salt 11) against `standing / 200`; a tip writes
  the town's TRUE HOLD count at the town centre, jittered up to 100 m per
  axis, into the sighting table, and a fresher soldiers' report wins over
  it. Accepted as D9-legal per 7.3 ("a local talked");
- the player's position;
- `AlertMachine`'s player-position fallback.

The simulation (the drain, `Fight`) still resolves on true presence.

`gmOccupierIntel` returns the sighting table, so tests can pin D9.

### 6.2 Posture and the first town

- **DORMANT.** While fewer than 2 DISTINCT towns have ever been taken,
  `Decide` answers flips as `4400fe9` does. The distinct set is a saved name
  set, `_townsEver`.
- **AWAKE.** The flip that brings the set to 2 goes to the director. It is
  latched.

**Invariant W.** Before then the director considers nothing. A re-flip of
town 1 while DORMANT is answered by `Decide` at 1.0.

### 6.3 Candidates, tokens and urgency

**Heat candidates (towns).**
- Each held town carries a fixed roll per flip,
  `u = Roll01(MakeSeed(town, flipDay, flipMinute, 0), 7)`.
- It is a candidate while `heat / 40 >= u`.
- Every resistance-held zone (towns AND bases) decays at 0.25 per 10 s
  (`gmHeatDecayHeld`), so a quiet flip (40) is remembered for about 27
  minutes and a loud one (about 55) for about 37.
- A RED alert at a held town, raised by occupier units there that see the
  player, adds heat like any edge and makes that town urgent. It is a
  candidate, not an alarm answer.

**Base candidates (D16).** Only while STRONG, resistance-held bases by
the same heat rule (their flip also gave +40). A quiet base is also reached
through push-out (6.4), so a strong occupier goes for bases whenever it is
strong, not only after a fresh flip.

**HQ candidate (D13, D14, D24).** Only with an established HQ, no HQ
record and no "@HQ" cooldown. While STRONG or in a wave (a wave inbound, or
landed within 1800 s: an accepted deviation) it uses the heat of the zone
the HQ stands in; an HQ outside every zone is never a heat candidate. It
becomes a candidate unconditionally, and first, once the resistance owns no
base and no town besides the HQ's own zone (`FinalPush`; the exemption flag
is not checked): the final push. The operation is always an HQ_ASSAULT on
`HqPos` (9.1), minimum `forcesHqAssaultMin` (6) bodies.

**Tokens.**
- 0..2 tokens, full at the wake, refilled by band: one per 600 s while
  STRONG, 1050 s STRETCHED, 1800 s THIN, none SPENT
  (`forcesTokenInterval[]`; until 2026-10-15 {1800, 2700, 3600, 0}, still
  reachable by authoring the array).
- Each 60 s tick with a token and a candidate launches one operation on the
  first candidate. The order (2026-10-16, video 5): urgent first, then by
  whole heat points, then by VALUE, then by heat. The value weight is the
  tiebreak within the same urgency and heat: a base 3, a town within 1 km
  of an occupier-held base 2 (`adjacentTown`), a far town 1, the HQ 2
  (`OccupierForces::CandidateValue`). Video 5 is the case it settles: Le
  Port (0.5 km from its Harbour) and Arudy (2.2 km from the Outpost) were
  both re-armed at 40, and Le Port goes first. The launch writes one line
  naming the weight: "[op 13] candidate Le Port, 1 of 2: a town by an
  occupier base (weight 2), heat 40". The final push stays first of all.
- At most `forcesOpsMax` (2) born operations are in flight, raised to
  `forcesOpsMaxStrong` (3) while STRONG (`OccupierForces::OpsMax`; the cap
  is the larger of the two keys, so `forcesOpsMaxStrong = 0` restores 2 in
  every band), plus one per living wave record (`CounterAttack::OpsCap`);
  wave operations take no token.
- A repelled operation puts its target on a 30 min cooldown (+- 20 %,
  below), with +15 heat
  unless a RED edge already paid it.
- **Re-arm at the end of a rest** (2026-10-16, round 3.5, video 4). When a
  rest ends on a zone the resistance still holds (a town or a base, never
  the HQ's zone) and the director's sighting table still reports fighters
  there (its men's reports or an informant tip, 6.1), the zone is
  RE-ARMED: on every director tick its heat is held at least
  `forcesRearmHeat` (40, the candidate heat `forcesHeatFull`), so
  `heat / 40 >= u` for every roll and it stays a heat candidate. The re-arm
  ends when an operation is launched on it, when its reports decay (it is
  quiet then, and push-out reaches it while STRONG, 6.4) or when it changes
  hands. One INFO line per re-arm ("Arudy rested with 3 fighter(s)
  reported: re-armed, heat held at 40 until an operation goes"); the names
  are saved ("Rearm") and listed by `gmOccupierForces` [30];
  `forcesRearmHeat = 0` restores the old rule.
- **The rest's jitter** (2026-10-16, video 5). Every rest, after a repel or
  an unborn cancel, is `forcesTownCooldown x (1 + 0.2 x (2u - 1))`, u a
  hashed roll of the zone and the operation id (salt 13,
  `OccupierForces::JitteredRest`): 1440 to 2160 s by default, the same rests
  on a replay, a different one for each operation. The re-arm comes at the
  rest's end, so the re-arm launch carries the same jitter; no second delay
  is added (the re-arm launch had none to jitter). In video 5 every answer
  to Le Port came 1800 s plus a token's wait after the last repel (241,
  2643, 4565, 6487, 8409 s) from the same birth point, which a player learns
  to the minute. The share is the `restJitter` lever (0..0.5, 0 = the old
  clockwork); the test lever `gmForcesForce ["cooldown", ...]` rests a zone
  for exactly the seconds given.
- **The repel ladder** (2026-10-16, video 5: Le Port got the same Ural and
  T72 from the Harbour seven times, Arudy the same BMP from the Outpost five
  times; 11 of 22 operations were repelled and nothing changed). Per zone
  the director remembers the repels of its own operations (town and base
  retakes, any origin: director, push-out or wave; never an HQ assault),
  with the last repelled package and the base its infantry came from. The
  memory is saved ("Repels") and forgotten when the zone changes hands
  (either way). Each rung writes one INFO line naming the count and the
  rule:
  1. *One repel, fighters reported:* the next director or push-out plan on
     the zone needs `L_req x 1.5` (`escalateLethality`) and at least
     `2 x knownFighters` infantry (`escalateInfantry`, one group at most),
     so the assault is infantry, not hulls alone ("Le Port: first repel
     (TRUCK from Le Port Harbour, 4 fighter(s) reported), next attack needs
     lethality x1.5 and at least 8 infantry"). With nobody reported the
     rung waits for a report.
  2. *Two repels* (`approachRepels`): a different approach. The plan
     excludes the last source; if no other base can staff it, an insertion
     (after a ground answer) or a ground answer (after an insertion) from
     any base. If neither can go the plan masses (below). A strike may go
     with this operation if the pool stays STRETCHED (7.3, the escalation
     strike; decision 3 of the feel assessment). Line: "Arudy: second repel
     (APC from Outpost), next attack from a different source or by air; a
     strike may go with it if the pool stays STRETCHED". The launch adds
     "[op N] the repel ladder on Arudy (2 repel(s)): lethality 16.20
     needed, at least 6 infantry; a different approach: TRUCK from Le Port
     Harbour instead of APC from Outpost".
  3. *Three repels* (`containRepels`): CONTAIN, do not attack. The zone is
     marked contained (saved): it is no heat candidate, no push-out target
     and never re-armed, and the director spends nothing on it. Instead the
     nearest occupier-held base on its landmass is surged by the bodies the
     next operation would have needed (`max(retake minimum, 2 x fighters)`,
     at most 12), a pool-funded deposit that may stand above garrisonMax
     ("Arudy: third repel (...), contained: no more attacks on it; Outpost
     surged by 6"). The containment lifts, with one line each, when the
     zone's reports decay (it is then quiet, and push-out may reach it,
     6.4), when a wave fires (7.1), or when control changes band (crosses a
     `forcesWaveThresholds[]` entry either way). The count is kept, so a
     further repel contains it again.
  `gmOccupierRepels` lists the rows `[zone, repels, lastPackage,
  lastSource, contained]`; the test lever `gmForcesForce ["repels", zone,
  n]` sets a count (containment from `containRepels` on, no surge).
- **Massing instead of trickling** (2026-10-16). When the ladder has shaped
  a director candidate's plan and the plan is under strength (the escalated
  infantry above every base's spare, the second rung blocked, or no package
  meeting the escalated `L_req`), the director holds its token and launches
  nothing that tick ("massing for Arudy: need 6 infantry, best source
  spares 5 (the token is held, tick 1 of 3)"). The held target is weighed
  first on every tick. The hold ends when a base can staff it (the full
  plan goes, "massing for Arudy ends: Le Port Harbour can staff it"), when
  a further token arrives during the hold, or after `forcesMassingMax` (3)
  ticks: then the strongest package one base can bring goes (the minimum
  lowered to the largest single-base spare, any different approach that
  can go). A target that stops being a candidate drops the hold. A first
  attempt never masses: until a repel the director has no reason to think
  the minimum is not enough. `gmOccupierMassing` gives `[target, ticks,
  forcesMassingMax]`; massing is not saved (re-weighed after a load).
  - *Deviation from the brief.* "When two tokens are held" is read as a
    second token arriving during the hold: the purse is full (2) after any
    quiet spell, so the literal rule would end every hold at once.
- **The ladder's numbers are constants**, not config keys: each has a
  `gmForcesTune` lever of the same name for tests; the lever round kept
  them constant (section 16): `escalateLethality` 1.5, `escalateInfantry`
  2, `approachRepels` 2, `containRepels` 3, `massingMax` 3
  (`forcesMassingMax`), `restJitter` 0.2, `adjacentTown` 1000 m; the value
  weights 3 / 2 / 1 / 2 and the jitter salt 13 are `OccupierForces`
  constants.
  - *Why.* In video 4 Arudy (3 fighters) repelled op 5 at 2429 s and op 7
    at 3749 s. The +15 decayed inside the 1800 s rest (heat 0 from 4717 s),
    while 24 informant tips kept the town from ever being quiet, so from
    7011 s to 8297 s a STRONG director with 2 tokens and 2 of 3 slots free
    had no candidate and Arudy stayed held.
  - *The choice.* Of the three fixes considered, the floor at the rest's
    end keeps D9 (it reads only the director's own rest and its sighting
    table, never the true census) and 6.4 (push-out stays the creep onto
    QUIET zones, and still takes a zone whose reports decayed). Freezing
    the repel heat during the rest keeps only what the town had at the
    repel plus 15: a town that had cooled before it was attacked (Arudy at
    op 7) still falls below its roll. Making push-out take "held, reported,
    rest over" zones works only while STRONG and sends the minimal-package
    rule at a defended town.
  - *Deviation from the brief.* The brief proposed "the heat of one
    sighting" as the default. A sighting adds no heat in this model (a
    YELLOW edge adds 4, a RED edge 15), and any floor below 40 leaves the
    roll able to drop the town (15 lists it only for a roll up to 0.375),
    so the default is the candidate heat, 40.

**Cadence at a glance** (owner, 2026-10-15: "a more playable rate"). A
token buys one operation only when there is something to do: a heat
candidate (a re-armed zone included, above), or while STRONG a quiet held
zone for push-out (6.4). Alarm answers (8.2) and wave strikes (7.1) take no
token and come on top. Since 2026-10-16 a push-out takes the arrival rule
like the director's other operations (6.4): it walks only when its foot
quote is under `forcesSlowEta` (900 s), so a quiet player's hour of
push-outs is no longer mostly walks (video 4: one walked 55 minutes from the
Airfield to the Outpost).

| Band (share of poolStart) | Token interval | Born cap | What the player sees per hour |
|---|---|---|---|
| STRONG (above 66 %, until below 60 %) | 600 s (was 1800) | 3 (was 2) | up to 6 operations, one about every 10 minutes, two at once after a quiet spell (a full purse); push-outs under the arrival rule since 2026-10-16 (a truck or an insertion past 900 s, foot under it) |
| STRETCHED | 1050 s (was 2700) | 2 | about 3.4, one every 15 to 20 minutes |
| THIN (below 33 %, until above 40 %) | 1800 s (was 3600) | 2 | 2, and none below `forcesPoolFloor` (20 %) |
| SPENT (0, until above 10 %) | none | 2 | none |

**The cadence against the pool** (the shipped Abel: poolStart 48, R 44
with the two generated outposts, G 56, so the refill is (48 + 44 + 56) / 4
= 37 units an hour, 5.2 and 5.6; STRONG is left under 28.8 and THIN is
entered under 15.84, the director's floor is 20 %, 9.6). What a STRONG hour
of six operations debits:
- *Foot.* Nothing at launch: the source base pays the infantry from its
  stored reserve (5.3). The pool pays it back through D2 regrowth, at most
  1 body per 600 s per base, so at most 6 bodies per base per hour whether
  the squad died or settled as a garrison. Abel's five ground sources
  (Outpost, Harbour, Airfield and the generated La Riviere Outpost and Vigny
  Outpost, 5.7) draw at most 30 an hour.
- *Truck.* Hull 3 + driver 1 = 4 units while it is out, refunded when the
  hull is deleted at home unseen, lost only when it is destroyed.
- *Insertion* (the arrival rule, 6.5; push-outs too since 2026-10-16).
  Mi17 10 + crew 2 = 12 units (17 off-map), refunded when the airframe is
  removed at home, lost when shot down.

Worst hour of foot and truck operations (six trucks, every hull destroyed,
every squad lost): 6 x 4 + 18 = 42 units against the refill of 37, so the
pool falls by 5, from 48 to 43, and stays STRONG. The band reads the
strength (5.6), which counts the same 42 lost units less what the refill
has repaid: 37 by the hour's end, so it too ends near 43; earlier in the
hour, before the refill has caught up, the unrepaid part is larger and 20
of it demote a full pool. Foot only: 18 against 37, the pool stays full. With
insertions in the mix, four trucks and two insertions all lost cost 16 +
24 + 18 = 58: the pool falls from 48 to 27 in the hour, under 28.8, so
STRETCHED by the pool alone, and the next hour runs at the STRETCHED rate.

**One ground launch could take most of the pool** (videos 6 and 7, until
2026-10-17). Trucks and insertions are the cheap end. The chooser buys the
cheapest package that meets the lethality required and the ETA (6.5), and
when none meets it (the repel ladder's x1.5, the end of massing) it takes
the most lethal one; until 2026-10-17 the only checks on its price were
`Spend`'s (units <= pool, and `CanOperate`, which refuses only SPENT, or
THIN under the 20 % floor). An APC with a tank escort is 8 + 3 + 14 + 3 =
28 units: at 2823 s in video 6 op 4 ("APC (6, BMP) with T72 from Le Port
Harbour to Sainte Marie ... the pool pays 28, ... short of the required
lethality: the most lethal affordable package") took the pool from 32 to 4
in one launch, STRONG to THIN and under the floor, and video 7 repeated it
at 2884 s (op 4, the same package, the strongest one base could bring
after three ticks of massing, the pool again 32 to 4, STRETCHED to THIN):
the director launched nothing more until op 3's T72 and Ural came home and
refunded the pool, and the occupier answered nothing for most of an hour.
In flight at once: at most three operations (`forcesOpsMaxStrong`). When
the pool is not the limit, what stops the director first is spare infantry
(the three ground bases hold about 20 spare bodies when full and regrow 18
an hour), which the refusal line names (6.5).

**The reserve floor** (owner decision 2026-10-17, video 7 op 4; the aim is
realistic behaviour: a commander keeps a reserve and does not throw it at
one village).
- **The rule.** A director-bought ground operation may not take the pool
  below `forcesPoolFloor` x poolStart (0.2 x 48 = 9.6 on Abel) by its own
  debit. The test is made at buy time, on the pool AFTER the launch's price
  (hull, crew, both hulls of a column; the infantry comes from the source's
  stored reserve, not the pool), compared as shares like `CanOperate`
  (`OccupierForces::KeepsReserve`, `ReserveRoomOf`): with 48 the most one
  launch may cost is 38, with 32 it is 22. A launch the pool pays nothing for
  (foot) never debits the pool, so the rule never refuses it.
- **Which operations.** The director's and push-out's retakes of a town or a
  base, whatever package delivers them (a helicopter insertion's airframe
  and crew included), every rung of the repel ladder, the massing fallback,
  a re-armed zone's operation, and an alarm answer to an occupier-held TOWN
  (a top-up of its garrison). `OccupierForces::ReserveBindsKind`.
- **Exempt** (they may dig into the reserve): the HQ assault (HQ_ASSAULT,
  the final push included), the final stand's surge, every wave operation
  (`CAOWave`: the strikes and the REINFORCE columns, paid from the wave,
  7.1), an alarm answer to an occupier base under attack (8.2), strikes (the
  opener, the escalation strike and the town strike keep their own band
  rules, `PlanStrike` / `PoolBandAfter`, 7.3: unchanged), the air re-plan
  of a watched ground operation (3.1, its own net ledger movement), the
  legacy dispatch and the test levers (`gmCounterAttackLaunchFunded`,
  `gmForcesForce ["alarm" | "strike", ...]`).
- **The plan does not shrink with the pool.** A floored plan is chosen on
  the most the pool could pay at max(pool, poolStart) with the floor kept
  (`ReserveBudget()`, 38 on Abel), not on the pool as it stands, so its "most
  lethal affordable" fallback is the same package at a pool of 20 as at 48.
  Before the rule a low pool made the chooser pick a smaller fallback.
- **It waits; it is never sent undersized.** A plan that does not fit above
  the floor WAITS: no operation, no token spent, no rest (cooldown) set, the
  re-arm kept, the target still contained or a candidate as it was; it is
  weighed again on every later tick as the pool refills, and the director's
  loop goes on to its next candidate (no strike goes in its place). The
  plan's own cheapest QUALIFYING package (one that still meets the
  lethality the plan requires and the ladder's rung) may go in its place
  when that one fits (the plan re-made on the room the floor leaves,
  `PlanFor(..., ReserveRoom())`): an urgent or slow target whose arrival
  rule chose a dearer, faster package takes the cheaper one that meets the
  same requirement, and the launch writes one line ("[op N] the reserve
  floor: APC (8, BMP) with BMP (the pool pays 22) would leave the pool under
  its reserve (pool 31 of 48, floor 9.6); TRUCK (8, Ural) with T72, which
  meets the plan, goes (the pool pays 21)"). A plan already short of its
  lethality (the most lethal package) has no cheaper qualifying package: it
  waits whole. A town that repelled two attacks is not sent an undersized
  third.
- **The massing fallback.** When massing ends (6.3 above) and the strongest
  package then waits on the floor, the target is not massed again: each
  later tick re-plans the same fallback until the pool can pay it.
- **One line per wait** (D12, 6.6), when the target's plan or reason
  changes, not every tick: "OccupierForces: Sainte Marie: escalation retake
  after massing (APC (6, BMP) with T72, the pool pays 28, lethality 25.50 of
  32.40, the most lethal package) waits, the pool keeps its reserve (pool 32
  of 48, floor 9.6: the launch would leave 4)"; a pool short of the price
  reads "the pool is N short of the price". The waits are listed by
  `gmOccupierReserve` (6.6) and are not saved (a load logs the wait once
  more).
- **Video 7's op 4 under the rule.** The pool at 32 keeps 22 to spend: the
  APC and T72 (28) waits, and so does every cheaper package (the truck and
  T72 at 21 would fit but brings 19.5 of the 32.4 needed, an undersized
  attack). The refill (37 an hour at full standing, 5.2) and op 3's hulls
  coming home lift the pool past 38 within minutes, and the whole package
  goes, leaving 10; meanwhile the 22 units in hand still pay for a cheaper
  operation elsewhere or an alarm answer. Tests: the reserve-floor
  `[occupierforces]` cases (`test_occupier_forces_reserve.cpp`) and the tri
  test `guerrilla_forces_reserve_floor` (the video's package on a Le Port
  held by six fighters: it waits at 32 and goes at 38).
- `forcesPoolFloor = 0` turns the rule off (and the THIN floor of 5.6 with
  it); the old pick and price checks come back.

**Urgency (with hysteresis).** A target is URGENT from either of these until
its heat falls below 35:
- an alert edge (YELLOW or RED) on it since its flip, or an alarm;
- heat >= `forcesUrgentHeat` (50).

A fresh quiet flip (40) is therefore not urgent, whatever the tick phase.

### 6.4 Push-out (D8)

When STRONG with a free token and no heat candidate, the director retakes a
QUIET town or base: the held zone nearest an occupier base with no sighting
for 30 min, under the usual cooldown. It uses the cheapest package (a
push-out is never urgent) under the arrival rule (6.5), like the director's
other operations (2026-10-16, round 3.5; the owner's "a more playable rate";
video 4: a push-out walked 55 minutes from the Airfield to the Outpost). A
push-out whose foot quote is under `forcesSlowEta` (900 s) still goes on
foot; a slower one may take a faster package within `forcesSlowCostShare`
(3.0) x the cheapest, and past `forcesAirEta` (1500 s) a helicopter
insertion. Until 2026-10-16 the arrival rule did not apply to a push-out,
which crept back on the minimal package whatever the walk. A zone
re-armed after a rest (6.3) is a heat candidate, not a push-out target, so
push-out stays the creep onto zones with nobody reported. A contained zone
(the repel ladder, 6.3) is never a push-out target; once its reports decay
the containment lifts and push-out may reach it, under the ladder's rungs
(a push-out plan escalates and changes approach like a director plan). The spare cap
(7.2) applies too. D2
regrowth refills posts at the same time, so a quiet player watches the
occupier creep back.

### 6.5 Packages, prices, lethality (D1, D2)

**Hull classes** are classified natively by the plan-15 pass from each
faction's `vehicles[]` ladder plus config:

| Class | Rule |
|---|---|
| Truck | ground vehicle, `transportSoldier` >= 6, no crew weapon |
| Armed car | wheeled, crew weapon |
| APC | tracked or wheeled armour with `transportSoldier` >= 4 and a weapon |
| Tank | tank simulation, main gun |

New descriptor keys (`forcesTruck`, `forcesArmedCar`, `forcesApc`,
`forcesTank`, `forcesTransportHeli`, `forcesAttackHeli`, `forcesJet`)
override that classification. Air must be authored.

**Precedence:** APC is tested before Tank, so a BMP (gun +
`transportSoldier` 8) is an APC.

Examples:
- **WEST:** the Jeep is not armed (no armed car); Truck5t is the truck; the
  M113 is the APC; the M60 is the tank.
- **LoBo Hizballah** `{Ural, JeepMG, Ural zsu23, JeepAT}`: Ural is the truck;
  JeepMG and JeepAT are armed cars; Ural zsu23 is an armed car (an AA
  truck, no cargo); there is no APC or tank.

A package missing for the faction is simply not offered. No war-level gate
applies; war level picks the infantry tier.

**Armour packages (phase 5).** Every ground package (foot, truck, armed car,
APC, tank) is offered by default (`ForcePackages::DefaultPackages`); air
waits for phase 6. A test can still pin the phase 3 ladder
(`Phase3Packages`).
- **Crew.** What the class needs: `hasDriver + hasGunner + hasCommander`, at
  least a driver. Paid by the pool as bodies on top of the hull. The spawn
  seats a driver, a gunner where the hull has one and a commander where it
  has one; crew without a seat stay home and go back to the pool.
- **Seats.** A hull carries at most `min(12, transportSoldier)` infantry. A
  hull whose seats cannot hold the minimum bodies is quoted (`seatsShort`)
  but never chosen, so a tank (no cargo) or a two-seat armed car does not go
  to a retake on its own.
- **Columns.** A quote may be a CARRIER (truck or APC: it carries the
  bodies) plus at most ONE ESCORT hull (armed car, APC or tank: it carries
  nobody). Foot never takes an escort.
  - Price: bodies + carrier + escort + both crews (crews paid as bodies).
  - Lethality: the carrier's (carried infantry x 1.25, an APC carrier's +6)
    plus the escort's bonus (+3 / +6 / +12). An APC escort adds its +6 and
    carries nobody.
  - Abstract speed: 0.6 x the slower hull's `maxSpeed`.
  - The choice runs over packages and columns alike, with the same rule.
    This is how a tank or a small armed car is bought: as an escort.
  - The launch INFO line names the escort after the carrier (2026-10-16,
    video 4, whose op 1 line hid a 17-unit T72 inside "the pool pays 21"):
    "[op N] Motorized squad (4, Ural) with T72 from Le Port Harbour to Le
    Port (...)"; base retakes, HQ assaults and alarm answers alike.
- **At the town edge.** The infantry dismounts and assaults. An armed hull
  (armed car, APC, tank, and every escort) then supports: it drives up to
  the town edge with its crew AWARE and firing. A truck holds back where it
  stopped (`ForcePackages::EdgeRole`).
  - **Arrival births** (round 3.5, 2026-10-16, after video 4). An operation
    that arrives abstract at a target with defenders is born live at the last
    route point at least `counterAttackBirthMinToTarget` (250 m) from it,
    under the unseen rule, and a hull package is born there exactly like a
    route birth: the carrier with its crew and the infantry seated in it,
    the escort `EscortGap` behind it with its own crew. The carrier drives
    to the 250 m mark and the infantry dismounts; the escort drives up to
    the town edge and supports, an armed carrier too, a truck holds back.
    So the lethality the director paid for is at the edge. The hulls count
    against `forcesLiveHullsMax` and every live budget like any birth: over
    any of them the arrival waits at the edge, abstract, and is retried
    every 30 s (`CounterAttack::RetryInterval`); it is never born on foot to
    fit. Only a foot package, a hull already lost or settled, or a hull the
    world could not create (the existing route-birth fallback: on foot, the
    price refunded) comes on foot. A helicopter insertion that arrived
    abstract keeps its old rule: its infantry is born on foot at the edge
    and the airframe goes home abstract. The ledger settles each hull one
    for one as on a route birth (`CAOpHullRefund` home unseen,
    `CAOpHullLost` destroyed or abandoned, `CAOpCrewSettled` for every
    crewman). An arrival-live force never returns to the abstract road.
    Tests: the three "arrival-live" `[counterattack]` cases and the tri test
    `guerrilla_forces_arrival_armour` (a Ural and BMP column on a Houdan
    held by three fighters, the player in the western hills).
  - **The stall rule** (video 6, 2026-10-17: op 13's Ural, born at the
    arrival point 306 m from Sainte Marie on a cross-country leg, never
    answered its driver's Move there and sat with nine men seated for
    1617 s; the dismount was keyed on the distance alone). A mounted force
    that has not closed `CounterAttack::StallProgress` (20 m) on its
    target for `StallNearTime` (45 s) within `DismountReach` (100 m) past
    the 250 m mark, or for `StallTime` (90 s) anywhere on its route,
    dismounts where it stands (`CounterAttackMarch.cpp`). Near the mark
    the infantry assaults from there and the live timeout starts, as at
    the mark; farther out it walks the rest of its route and deploys at
    the town edge like a foot squad. A truck holds back where it stuck, an
    armed hull still supports. The clock also runs while a carrier waits
    for its escort, so an escort that never closes up cannot hold the
    infantry seated. One INFO line names the hull, the seconds and the
    distance. Tests: the stall `[counterattack]` cases and the tri test
    `guerrilla_forces_stuck_carrier` (the video's Ural and T72 from the
    Outpost on a Sainte Marie held by six fighters).
  - **Break-off** (decision 4, the feel assessment's item 7, NEXT question
    2; after video 5, where hulls drove home while their riflemen died and a
    lone survivor stood 47 m from Arudy for 18 minutes). A live RETAKE,
    BASE or HQ_ASSAULT whose living infantry falls below a third of the
    strength it was first born with (`CounterAttack::BreaksOff`: alive x
    `BreakOffDivisor` 3 < born; `Record::bornStrength`, saved) while the
    target is still the resistance's withdraws at once, on the road or at
    the edge, any band. The lone survivor is the same rule. Top-up columns
    and alarm answers never break off (their target is the occupier's).
    With `occupierForces` on or a funded record only.
  - **Withdrawal** (break-off, `counterAttackLiveTimeout` at the target,
    the HQ it was sent at moved). The record settles at once: the losses
    booked, the terminal `CAOpRepelled` with its reason
    (`CounterAttackOpEvent::reason`, `CAERBrokenOff` / `CAERWithdrew`; a
    wiped force is `CAERDefeated`), so the director rests the target and
    counts a repel. The survivors walk back toward their source (the
    nearest occupier base if it fell) as a withdrawing Leftover: re-sent
    home every 30 s, removed only beyond 850 m of the player and unseen,
    then home as bodies room-capped (CrewHome), the dead among them lost.
    The live timeout withdraws this way too, in the open, with the player
    near or not (it no longer waits for him to leave). Its clock starts at
    the town edge, or earlier once the road march's allowance (the route
    left at 1 m/s plus 120 s, from the birth) is spent short of the edge;
    the withdrawal line then reads "counterAttackLiveTimeout 1200 s short
    of the town edge, the march allowance spent" instead of "at the
    target".
  - **The hulls leave after the infantry** (item 2). Infantry still seated
    rides home in its carrier (the HomeHull carries it as cargo; deleted
    unseen cargo first). Infantry on foot is waited for: every hull holds
    where it stands, an armed one still supporting, until no living
    survivor is within `ClearRadius` of the target (the larger of
    `counterAttackBirthMinToTarget` and zoneArea + 100 m: 250 m on Abel) or
    `CounterAttack::WithdrawHullWait` (120 s) has passed, then drives home
    and refunds as before. A top-up squad whose town was lost (M5) holds
    its hulls by the same two conditions. Each hull's price settles one for
    one as before (`CAOpHullRefund`, `CAOpHullLost`, `CAOpCrewSettled`).
  - **Lines.** Each withdrawal writes "CounterAttack: [op N] Broken off at
    X: L lost, M going home (born B, alive M; rule: ...)" or "Withdrew from
    X: ..." with where the survivors and hulls go and the player's
    distance; a held hull writes when it leaves and why ("cleared", "wait
    over", "no infantry"); survivors home unseen write one line. Tests: the
    break-off `[counterattack]` cases and the tri tests
    `guerrilla_forces_breakoff_column` (a Ural and BMP column, five of six
    riflemen killed, the hulls held until the survivor clears 250 m or 120
    s) and `guerrilla_forces_breakoff_lone` (one man of four left).
- **Downgrade.** A hull package's infantry is debited from the source's
  STORED reserve. If at launch that reserve cannot pay the bodies but the
  source can on foot, the package goes on foot: same source and bodies, no
  hull or crew, the pool price refunded. A column goes on foot whole, both
  hulls refunded (`MustDowngrade` / `Downgrade`, applied in
  `CounterAttack::LaunchCore`).

**Prices and lethality.** All prices are in pool units. Crew bodies are paid
as bodies on top of the hull price.

| Item | Price | Lethality |
|---|---|---|
| Infantry body | 1 | 1 x tier factor (1.0 / 1.25 / 1.5) |
| Truck | 3 | carried infantry x 1.25 (motorized) |
| Armed car | 4 | +3 |
| APC | 8 | +6; carried infantry x 1.25 |
| Tank | 14 | +12 |
| Transport helicopter | 10 (15 off-map) | inserted infantry x 1.25 |
| Attack helicopter | 18 (27 off-map) | +14 (strike) |
| Jet | 22 (33 off-map) | +16 (strike) |

**The choice.**
1. **Required lethality.** `L_req = 1.2 * counterAttackDefenderWeight (3) * knownFighters`.
2. **Minimum bodies.**
   - town and base retakes: `max(4, garrisonMax(target))`, capped at 12
     for a base (`min(max(forcesRetakeMin, garrisonMax), 12)`); after a
     repel with fighters reported, at least twice the fighters, and
     `L_req x 1.5` (the repel ladder, 6.3);
   - alarm answers: 4;
   - HQ assaults: 6.
   - A base retake or an HQ assault whose minimum no single base can spare
     takes the spare cap (7.2).
3. **Pick.** The cheapest package meeting both. An URGENT target takes the
   fastest package costing at most 1.5 x that. If none meets `L_req`, the
   most lethal affordable package goes, and its line says so. For an
   operation the reserve floor binds (6.3, 2026-10-17) "affordable" is what
   the pool could pay at max(pool, poolStart) with the floor kept (38 on
   Abel), and a pick that would take the pool under the floor waits, or
   gives way to the plan's own cheapest qualifying package when that one
   fits.
   - **The arrival rule** (owner, 2026-10-15; reworked 2026-10-16, phase
     1 C, after video 5, where all five insertions flew to targets with
     "0 fighter(s) reported" while every defended target got the cheap
     ground answer). A package's arrival is `PackageQuote::seconds` (the
     road quote below, the "road N s" or "line x1.5 N s" of the INFO
     line); T is `forcesSlowEta` (900 s). Ground first, air where it pays:
     - **Air.** A transport helicopter insertion goes only when one of
       these holds, and then at any price the pool pays (from the
       occupier-held airfield, else off-map at x 1.5): *across water*, no
       ground package can reach the target because every base that could
       pay stands on another landmass (`PackageSource::airOnly`, Malden's
       Southeast Compound); *past `forcesAirEta`* (1500 s), even the
       fastest lethal ground package needs longer (or no ground package is
       lethal); *fighters reported*, `knownFighters(target)` is at least
       `ForcePackages::AirFightersMin` (1) and the target is out of walking
       reach (the foot quote needs more than T). An empty target that a
       ground package reaches under `forcesAirEta` never gets air.
     - **Ground.** Otherwise the cheapest lethal ground package goes when it
       arrives within T. When it is slower, the CHEAPEST lethal ground
       package that arrives within T goes instead, never the fastest one
       within a price share. `forcesSlowCostShare` (3.0, now the constant
       `ForcePackages::SlowCostShare`; the key and
       `PackageRequest::slowCostShare` are kept for tests) is only an upper
       bound on what that package may cost against the cheapest one. In
       videos 4 and 5 the share never decided a pick. When nothing within
       the bound arrives within T, the fastest within it goes.
     - **URGENT.** Within T an URGENT target takes the fastest ground
       package costing at most `forcesUrgentCostShare` (1.5) x the cheapest
       (for a 4-body retake that is still foot: a truck costs 2 x). Past T
       it binds every time: the FASTEST lethal ground package that arrives
       within T replaces the cheapest one that does. Urgency never buys air
       on its own.
     - **Sourcing.** A package's infantry comes from the base with the
       shortest leg whose spare covers the bodies the package needs for
       `L_req`; when no base can, the nearest whose spare covers the
       minimum (the quote is then short). A defended Goisse thus flies the
       Airfield's 9, not the nearer Outpost's 5.
     The rule needs the transport helicopter in the catalogue (the faction
     authors `forcesTransportHeli`) for the air cases, and it keeps
     `L_req`, the minimum, the pool's affordability and the stored-spare
     rule for hulls. It applies to the director's heat candidates, base
     retakes and HQ assaults (origin `CAODirector`), to push-outs (origin
     `CAOPushout`, since 2026-10-16, 6.4), to alarm answers outside THIN and,
     since 2026-10-16 (phase 1 A; video 5: the wave's HQ assault walked 41
     minutes, 2450 s by road, from the harbour that had just landed it), to
     wave strikes (origin `CAOWave`, 7.1): chosen from the hub within the
     strike's staged budget, the hull and crew paid by the pool like any
     package. The INFO words for the pick
     name the reason (`PackageChoice::reason`, `ForcePackages::ReasonWords`):
     "air: across water", "air: past forcesAirEta", "air: fighters
     reported", "ground: cheapest under slowEta", "ground: slow but no air
     case", "urgent: fastest under slowEta", "urgent: fastest within
     forcesUrgentCostShare". One INFO line names a changed pick: "[op N]
     arrival rule (ground: cheapest under slowEta): the cheapest, FOOT,
     needs road 1139 s; TRUCK goes, road 128 s".
   - **The road quote** (2026-10-16, after the comparison video: a base
     operation quoted at 1822 s straight arrived after 3393 s). A ground
     package's seconds are the leg the counter-attack service marches (the
     leg `CounterAttack::LegTime` times) over the package's speed:
     - the road route's length when one is known: the service's own search
       (`RoadRoute::Search` with the service's snap and join radii, from
       the base to the target, then the last link to the target), cached
       per (base, target) pair by `RoadQuoteCache` (`RoadRoute.hpp`). A
       route CounterAttack already planned for the pair is reused (its
       search records the length); otherwise the director plans the pair
       itself, at most once per tick (a failure is cached too) and at most
       6 pairs per tick. An entry lives 600 s of mission time; past that it
       is planned again when the tick has budget, else served as it is;
     - else the straight distance x `forcesRoadFactor` (1.5): no road route
       (a pocket base, no road near the target), a road past the service's
       detour rule (longer than 2 x the straight distance + 300 m: the force
       cuts across), or a pair past the tick's plan budget.
     Air (the helicopter insertion, the strike airframes) flies the straight
     line, unchanged. The source of a package is the base with the
     shortest leg for it (by road for a ground package, so a nearer base
     whose road winds the long way round loses to one with the shorter
     road) among those whose spare covers what the package needs (Sourcing
     above). One road factor serves the quote and the march:
     `ForcePackages::RoadFactor()` is the theatre's `forcesRoadFactor`, set
     by the director when it reads its tuning and passed as
     `PackageRequest::roadFactor` in every request, and the counter-attack
     march with no road route walks `ForcePackages::GroundLeg` with it, so a
     force walks the leg it was priced on (`counterAttackRoadFactor` 1.3
     goes). The INFO line names the quote it used: "road 1794 s", "line
     x1.5 2733 s", or "line 42 s" for air (`ForcePackages::EtaNote`; the
     last 16 are kept for `gmForcesEtaNotes`). Cost: a warm quote is one
     hash lookup; the unit timing case holds 6 sources x 14 towns under
     2 ms.
   - **On Malden** (foot 1.5 m/s, a Ural 13.3 m/s, a Mi17 53 m/s; a quiet
     4-body retake: foot 4, truck 4 + 1 + 3 = 8, Mi17 4 + 2 + 10 = 16
     units; road lengths from `gmRoadRoute` on Guerrilla.Abel, 2026-10-16,
     to the ground base with the shortest leg, link to link: the quote adds
     the last link to the town centre, a few hundred metres more). Le Port
     (0.5 km of road from Le Port Harbour), Dourdan (0.7 km from the
     Outpost), the Village (the Outpost's road winds 1.5 km for 0.5 km
     straight, past the detour rule: line x1.5, 0.8 km) and Saint Louis
     (1.3 km from the Airfield, about 880 s) get foot. Every other empty
     town gets a truck: La Trinite (1.7 km by road, about 2 minutes), Arudy
     (3.0 km by road to the centre, "road 1994 s" on foot, about 225 s by
     truck), and Larche, Chapoi, Sainte Marie, Cancon, Goisse, La Pessagne,
     Vigny and La Riviere (by road from 2.9 km for Chapoi to 6.5 km for La
     Riviere: foot 32 to 72 minutes, a truck 4 to 8 minutes, all under
     `forcesAirEta`). Until 2026-10-16 those ten flew. A town out of walking
     reach with fighters reported gets the insertion: Goisse with three
     (`L_req` 10.8) is a Mi17 with the Airfield's 9 (21 units) where the
     Airfield's truck would need about 9 minutes. The Southeast Compound,
     an island, is always an insertion.
   - **The refusal names the shortfall.** With no source able to spare the
     minimum, the line reads "no occupier base can spare the minimum (12
     needed, the largest single-base spare is 6: short 6)".
4. **Grant (review M1).** The director quotes each source's STORED spare
   (minus a wave surge), the same figure `CounterAttack::LaunchCore`
   grants from; a funded order is granted only in full from that reserve,
   else refused. A refused source is zeroed and the launch re-planned on up
   to 4 other sources (`MaxSourceRetries`), stopping if the pool refused.

**Resolution: always a real fight against present defenders.**
- **Defenders are always live.** Every resistance body is live AI wherever
  it is: fighters left in a town, hold squads at a base. So with forces on,
  any arrival at a target with defenders present goes LIVE, born under S
  even off-screen, for towns as well as bases.
- **No abstract `Fight` with the key on.** A won abstract roll would leave
  the player's fighters standing in a reverted town.
- **Over budget,** the force waits abstractly at the town edge and does
  nothing to the town.
- **Abstract resolution remains** only for EMPTY targets: hold and drain.
- **The legacy `Fight`** (key off) keeps its two quantities apart:
  `A_lethal` for the odds and `A_bodies` for the losses.
- **What `L_req` is for.** It sizes the package; the live fight decides the
  result.

A strike (jet or attack helicopter) on a fighter target is bought only as
an add-on to a ground operation. Strikes on a town itself follow 7.3.

**Worked examples (Abel, tier 1):**
- **Quiet empty town 1 km from a base.** Not urgent; minimum 4 bodies.
  - Foot: 4 units, about 15 min.
  - Motorized: 4 bodies + a driver + a truck = 8 units, about 2 min.
  - Choice: **foot**.
- **Loud flip (RED edge), 3.5 km from the Harbour, 3 fighters reported.**
  Urgent; `L_req = 10.8`; minimum 4.
  - Foot 11: 11 units, about 39 min.
  - Motorized 9 + a driver: 13 units, lethality 11.25, about 5 min.
  - APC 4 + its crew (2 for an M113, 3 for a BMP) + 8: 14 or 15 units,
    lethality 11.0, also within 1.5 x 11 but slower than the truck.
  - The tank carries nobody, so it cannot bring the minimum 4 alone.
  - Columns: truck + APC 4 + a driver + 3 + 2 crew + 8 = 18 units,
    truck + tank 25 units; both above 1.5 x 11 = 16.5.
  - 13 <= 1.5 x 11 and the truck is the fastest, so the choice is
    **motorized**.

**Group budget.** Each package counts its groups against the 40-group budget:
infantry 1, each crewed hull 1, a helicopter insertion 2.
`counterAttackMaxLive` counts live infantry records; strikes count as
groups, not as live squads. Live hulls also count against
`forcesLiveHullsMax` (6) for frame time, because the AI census counts
crews, not hulls. A launch over budget waits like a live squad does.

### 6.6 Logging and diagnostics (D12)

**One INFO line per action.** Every line starts with `OccupierForces: [op N]`
(N is the `gmOccupierOps` id). Examples:
- "OccupierForces: [op 12] Motorized squad (9, Truck5t) from Southeast Compound to Larche"
- "... Foot squad (6) from Le Port Harbour to Houdan"
- "... Helicopter insertion (8) from off-map to Vigny"
- "... Jet strike over Houdan (town)"
- "... HQ assault (APC squad) from Airfield on Camp"
- "... Base retake (foot 8) from Airfield on Le Port Harbour"
- "... La Riviere Outpost retaken: 6 stay as its garrison" (the settle of a retake: the zone was lost and is the occupier's again)
- "... Le Port Harbour reinforced: 0 joined its garrison; 0 back at Airfield, 2 to the pool (pool 50 of 48, 2 over poolStart in the wave headroom)" (the settle of a REINFORCE top-up or, as "alarm answer settled", of an alarm answer that topped up: the zone was never lost, so never "retaken"; it names the bodies that joined, those back at their source and those credited to the pool, banked or dropped, and the wave headroom when the pool stands over poolStart. Round 5: video 6 wrote "[op 11] Le Port Harbour retaken: 0 stay as its garrison" for a top-up whose 2 bodies went to the pool unannounced)
- "... Alarm answer: APC squad from Airfield to Outpost"
- "... Cancelled unborn: every route watched"
- "Wave 1 (30) inbound to Airfield, control 0.74"
- "Push-out: foot squad from Outpost to Goisse"
- "CounterAttack: [op 12] Broken off at Arudy: 3 lost, 1 going home (born 4, alive 1; rule: living infantry below a third of the strength born, the target still held); on foot toward Outpost, 47 m from the player" (the movement side's withdrawal line, 6.5 "At the town edge")

"Starved: no answer to X alarm" is written once per alarm edge.

A launch that waits on the reserve floor (6.3, 2026-10-17) has no op id
(nothing was launched, no id is spent) and writes one line per target, again
only when its plan or the reason changes: "OccupierForces: Sainte Marie:
escalation retake after massing (APC (6, BMP) with T72, the pool pays 28,
lethality 25.50 of 32.40, the most lethal package) waits, the pool keeps
its reserve (pool 32 of 48, floor 9.6: the launch would leave 4)". The words
name the operation ("retake", "base retake", "alarm answer", with
"escalation", "re-arm" or "push-out" in front and "after massing" behind
when they apply) and the plan as the launch line would. When the plan's own
cheaper qualifying package goes instead, the launch adds "[op N] the
reserve floor: <plan> (the pool pays P) would leave the pool under its
reserve (pool X of S, floor F); <package>, which meets the plan, goes (the
pool pays Q)".

The band line ("band STRONG -> STRETCHED (pool P of S; strength N: ...)")
is written at most once per frame, when the frame's one step changes the
band, plus one for a wave landing or a forced pool, which step at once
(5.6, "When the band is stepped"): a debit and its matching credit
booked in one transaction never write a pair.

**Dev-only since 2026-10-14 (owner decision).** These per-action lines, and
the same INFO trail of `CounterAttack` / `CounterAttackAir` ("CounterAttack:
..."), `OccupierAir` ("OccupierAir: ...") and the director's startup lines
(the sizing line "OccupierForces: active, poolStart ...", the settlement
probe lines, the package catalogue, the HQ-destroyed line), log at INFO only
while the director log toggle is on, and at DEBUG otherwise, so
`--log-level debug` still shows them. The toggle is on when the game runs
with `--dev` (`AppConfig::DevMode()`) or the mission authors `forcesLog = 1`
in CfgGuerrillaZones (`gmForcesTune ["log", 1]` turns it on at run time).
One helper carries it: `GM_DIRECTOR_INFO` / `DirectorLogLevel()` in
`DirectorLog.hpp`. WARN and ERROR lines always log. Every tri test of the
layer passes `--dev`, so test logs keep the trail.

**Read-only commands:**
- `gmOccupierOps`: `[id, kind, origin, package, source, target, phase, bodies, hulls (0..2), eta s, birthReason, hullClass, escortClass]` per pending or active operation (13 columns; the last two are the carrier and escort class).
- `gmOccupierForces` (24 slots): `[pool, poolStart, band, tokens, posture, control, wavesFired, K, excludedTowns, lost, townsEver, urgent, cooldowns, opCounter, tokenClock, waveInbound, waveClock, waveBodies, waveName, finalFired, surgeZone, surgeBodies, wavesLanded, occupierWarLevel]`; band and posture "OFF" with the key off. `gmWarLevelOccupier` gives slot 23 alone.
- `gmCounterAttacks` (24 slots per record): `[town, source, phase, clock, strength, size, townsBefore, group, spawnPos, spawnReason, opId, kind, origin, package, liveHull, eta, hullClass, funded, hull, seatedInfantry, leader, livingInfantry, escortClass, escort]`.
- `gmCounterAttackEnds` (the break-off, 2026-10-16): the last 16 ended operations `[opId, target, type, reason, bodies, units, born, note, line]`, `reason` "defeated" / "broken off" / "withdrew" on a repel, `line` a withdrawal's "Broken off at X: L lost, M going home"; `gmCounterAttackHomeHulls` `[opId, hull, crewGroup, waiting, waited s, released, waitGroup, cargo, cargoLiving]`; `gmCounterAttackLeftovers` `[group, living, withdrawing, hull]`.
- `gmOccupierIntel`: the sightings `[zone, count, age s, positions]`.
- `gmOccupierReserve` (2026-10-17): `[floorUnits, room, budget, [[zone, what, plan, package, price, pool, best, line]...]]`: `forcesPoolFloor` x poolStart, the most a floored launch may cost now, what a floored plan is chosen on, and the targets whose plan waits on the reserve floor (6.3), with the pool when the line was written, whether it is the massing fallback, and the line.
- `gmRoadRoute [from, to, joinRadius?]` (the route with its join fields) and `gmRoadComponents [pos, radius]` (the road components near a point).
- `gmHqState` `[hasHq, destroyed, epoch, hqCapture, zoneIndex, pos]`, `gmHqEpoch`, `gmHqForceCapture`; `gmCampaignEnd` gains slot 8 `lost`, `gmCampaignLost` the latch.
- The air spike: `gmOccupierAir` and the levers `gmAirInsert`, `gmAirSortie`, `gmAirExposure`, `gmAirUnseen`, `gmAirAbort`, `gmAirReveal`, `gmAirTune` (column layout in `OccupierAirCommands.cpp`).

**Test levers:** `gmForcesTune [key, value]` and `gmForcesForce ["pool" | "wave" | "refillHours" | "cooldown", value]`. Test ranges reach down to 0.01 h. `gmCounterAttackLaunchFunded` launches a funded order directly (op ids from 900001; it spends with no ledger row, review L14).

There are no hints or journal lines from the director. How the director
reaches the journal is open: GearUnclear/CWR#98.

## 7. Control, waves, bases and town strikes

### 7.1 Control and waves (D7)

**Control.** `C = 0.6 M + 0.4 T`: M is the roster strength the occupier
still holds, T is the administered share of the K towns.

**Triggers.** C at or below 0.75, then 0.50, for two consecutive ticks, plus
the final stand when one roster base is left. Each fires once. Triggers due
together, or while a wave is inbound, merge. That gives three big moments.

**Wave size.** Half of poolStart (12..48). The lead is 10 min (2 min for the
final stand). A wave adds its bodies to the POOL. Each landed wave raises
the occupier-side war level by 1; resistance readers keep
`gmWarLevelTerritory`.

**Where a wave goes.** Score every resistance-held zone z:

`S(z) = sum over held zones y within forcesWaveCluster (2 km) of z of (4 if y is the HQ, 3 if a base, 1 if a town) + heat(y) / 40`

The best cluster is the target.

**Hub.** The named `forcesWaveHub`, else the occupier-held AIRFIELD, else
SEAPORT, else the strongest base. With no hub left the strikes depart from
the occupier base nearest each target on that target's landmass (`bcca80e`)
and no REINFORCE column goes (they need a hub). Phase 6 added no off-map
air for that case: wave strikes buy no air opener (an accepted deviation,
section 18).

The AIRFIELD and SEAPORT candidates include **seeded bases** (#76): a
template that authors no AIRFIELD and no SEAPORT zone gets the ones the
runtime detector finds (`seedBases`, ISLAND-PACKS.md section 10), seeded
occupier-owned and saved by name, so they are hubs like authored ones. A
template that authors either keeps exactly its authored hubs (seedBases
resolves to 0) unless it sets `seedBases = 1`.

**Allocation** from the pool, in order:
1. Hub top-up: a reserve deposit through `Deposit`.
2. Final-stand surge (below).
3. Strikes on the cluster: the HQ if in it, then bases, then towns.
   - Up to `forcesWaveRetakeShare` (0.5) of the wave's bodies as staged
     infantry, on up to `forcesWaveOps` (4) targets (origin `CAOWave`). The
     share counts the infantry only: each strike's hull and crew are paid by
     the pool on top, like any package (6.5), so a helicopter strike can take
     more than half the wave's units (video 7: 21 of 30, the share 15).
   - Each target gets its own package, from the hub, under the arrival rule
     since 2026-10-16 (6.5): a strike whose walk from the hub exceeds
     `forcesSlowEta` rides (video 5's HQ assault walked 41 minutes).
   - A wave that fires lifts every containment of the repel ladder (6.3):
     the parent nation's reinforcement is a reason to try again.
   - They launch together, only once the director is AWAKE (W); a DORMANT
     director logs "no strikes".
   - Paid from the wave, they are exempt from the reserve floor (6.3): a
     wave strike and a REINFORCE column may take the pool under
     `forcesPoolFloor` x poolStart.
4. REINFORCE columns on foot from the hub to the most-bled held bases, at
   most 12 bodies each and `forcesWaveOps` columns. A column joins its base
   up to the room the base has when it arrives; the rest goes home to the
   hub up to its room, and what neither takes is credited to the pool
   (5.5), above poolStart only inside the wave headroom. Its settle line
   says so: "[op 11] Le Port Harbour reinforced: 0 joined its garrison; 0
   back at Airfield, 2 to the pool (pool 50 of 48, 2 over poolStart in the
   wave headroom)" (6.6; video 6's column arrived 85 minutes after the
   wave, with both bases full again).
5. The rest stays in the pool (above poolStart: the wave headroom, 5.5).

**The wave surge** (decision 5, 2026-10-16). Video 5: after wave 1 every
base sat at `garrisonMax`, and the survivors and hulls that came home over
the ceiling were dropped (9 dropped, 1 banked). Now, while a landed wave
is on (`SurgeOpen`: the `WaveWindow`, 1800 s from the landing, or any wave
operation's ledger row still open), what the ceiling keeps out of the
pool is a SURGE: a pending deposit (S5) above `garrisonMax` at the occupier
bases nearest the wave's cluster (the anchor: `BestCluster`'s centre at
the landing, else the hub), each base up to `surgeShare` (0.5, a lever) x
its `garrisonMax` over it, the nearest first. One INFO line per base,
`surge: N unit(s) over poolStart at <base>, above its garrisonMax (...)`.
Nothing is dropped mid-fight; only what no base's surge cap can take falls
through to the bank rule (5.5), then to the drop. On Abel with every base
at `garrisonMax` the caps hold 22 bodies in all (Airfield 6, Le Port
Harbour 4, Outpost, Southeast Compound and the two generated outposts 3
each) [measured, tri `guerrilla_forces_surge` at the default share: a
23-unit jet home over the ceiling surged 22 and dropped 1]. Outside a wave the bank
rule of df9457e stays for every credit. The surge is ordinary reserve: the
director may spend it, and bodies spent or killed take their share of the
surge with them.
- **It decays.** Each base's surge leaves at the bodies it held at its last
  credit / `surgeDecayHours` per hour (1.0 h, a constant with the
  gmForcesTune lever "surgeDecayHours", read live), so the base is back at
  `garrisonMax` about an hour after its last credit. Only stored
  bodies leave (a live or pending surge waits). The leaving bodies rejoin
  the pool up to its ceiling; the rest stand down, one INFO line `surge: N
  unit(s) stood down at <base> (M back in the pool; ...)`, booked in
  `StoodDown()` (5.5). A surge at a base the occupier loses ends with it.
- The player sees the bases near the fight visibly thicker after a wave,
  thinning back over the next hour ("reinforce what it holds").
- Saved: the rows (`Surge`), the window, the anchor and `stoodDown`.

Wave operations take no command token. Each living wave record raises the
born-operation cap by one (`CounterAttack::OpsCap`), and a wave order gets
one more headroom at launch.

**Final stand.** The last base gets a surge of up to 12 above its strength.
The surge is never spare, and the last base never sources a strike. The
surge is excluded from spare only in the director's source choice (an
accepted deviation).

### 7.2 Base recapture (D16, D19)

**The meter.** A resistance-held military zone carries the mirror of the
player's capture, in its OWN field, `occCapture` (0..100, on `ZoneRecord`
and `ZoneSaveState`). It is not the player's `capture`, so `capture.sqs`'s
"Securing X" ticker, `ZECaptureStarted` and the marker labels never
misreport it as the player's progress.
- **Rising.** With occupier presence and no resistance HOLD presence it
  rises by `captureRate` (6, a per-zone override wins) x min(crew,
  `captureCrewCap` 3) per 3 s tick.
- **Frozen.** Both sides present freeze it.
- **Defended decay.** Resistance presence alone (the 3-man hold squad
  counts) decays it at `captureDecayDefended` (10 per tick), an accepted
  deviation from "resistance freezes it".
- **Decay.** With nobody present it decays at `captureDecayAbandoned`
  (2 per tick).
- **Who counts.** Any occupier unit (D19), so a patrol that kills the hold
  squad and stays takes the base in about 18 s.
- **Off-screen.** `abstractOcc` is extended from held towns to every
  abstract force of any kind holding the post, not suspended
  (`CounterAttack::AbstractPresence`), so off-screen base retakes tick.
- **Key.** `baseRetake` follows `occupierForces` unless authored (an
  accepted deviation); with `baseRetake = 0` held posts stay permanent.

**The minimum and the spare cap** (owner, 2026-10-15). A base retake asks
`min(max(forcesRetakeMin, garrisonMax), 12)` bodies from ONE source (6.5),
so on Malden the Airfield (12) could be retaken only by a wave once it
fell: the best spare left is the Harbour's 6 ("no occupier base can spare
the minimum", repeated through the director video's run 3). Now a base
retake or an HQ assault whose minimum no single base can spare takes the
largest single-base spare instead, never below `forcesSpareCapFloor` (6)
and never above the minimum (`OccupierForces::SpareCappedMin`): the
Airfield is retaken by 6 from the Harbour, a 6-man HQ assault stays 6. A
base on another landmass counts with its stored spare only when the
transport helicopter is offered. Below the floor it is still refused, and
the refusal names the shortfall (6.5). `forcesSpareCap = 0` restores the
one-source rule. The director logs which rule applies once per session
("base retakes and HQ assaults take the spare cap ..."), and each operation
the cap lowered gets a line ("[op N] spare cap: no base can spare the 12
bodies Airfield asks, 6 from Le Port Harbour ..."). A wave strike stages
its own infantry at the hub (7.1) and takes no cap.
- **Why not two sources.** The design step preferred two records from two
  bases on one target. `CounterAttack` keeps one record per target zone
  (`FindByTarget` refuses a second, "the target already has an operation"),
  one HQ_ASSAULT at a time, and the revert adopts that one record as the
  new garrison (`ZETownLost` / `ZEBaseLost`); events and the ledger row are
  keyed by one operation id (`FindByOpId`). A second record on the same
  target would need those rules rewritten, so the cap ships instead (D16
  and D19 fix no number).

**Strikes against live defenders.** A strike on a base whose hold squad
lives goes LIVE: hold squads are always-live, so the fight is real.
- Born under S, even off-screen.
- An abstract `Fight` against live defenders is never used, so a won roll
  never leaves the player's squad standing in an enemy base.

**Remote tick.** It is opened for resistance-held non-CITY zones with
occupier presence, as it is for held towns.

**At 100** (`RevertBase`):
- the owner returns to its authored occupier token;
- the garrison is settled (5.4): the holding force's bodies first, up to
  the room, the surplus to the pool, else a pool-funded deposit (pending
  inside the deposit bubble);
- the income tap closes (income 0);
- `lostCount` + 1 (saved; `capture.sqs` writes "Retaken" by it);
- a new `baseLost` zone event fires, with a serialized handler slot like
  `townLost`.

Once the campaign is won or lost `RevertBase` refuses; the meter itself may
still climb to 100 and sit there.

**Re-capture by the player.** It re-fires `captured`. The consumers are
audited:
- `capture.sqs` spawns a hold squad only if none of that zone's is alive.
  This needs per-zone hold-group tracking again: a zone -> group map in the
  style of the retired `GM_HOLD_GROUPS`. It writes "Retaken" instead of
  "Liberated" when the zone was held before;
- income: no change is needed. `economy.sqs` pays only
  OUTPOST/AIRFIELD/SEAPORT, and the capture only overwrites `income < 1`
  (a defensive check only);
- journal objectives are not re-completed;
- Traffic endpoints follow the owner automatically;
- `CampaignEnd::LastTaken` updates.

### 7.3 Town strikes (D17)

A jet or attack helicopter may strike a held town itself, not just reported
fighters.

**Live:** the pass is real. Whatever the aircraft's own sensors acquire is
engaged, civilians included.

**Occupier standing (D21).** The town model has only resistance support
today. This adds the smallest counterpart: a per-town `standing` (0..100,
default 50; an authored `standing` key per CITY; saved on the zone row).

- **Why per town and not campaign-wide.** Strikes, informants and
  garrisons are all per town, and a campaign-wide value could not say WHICH
  town turned on the occupier. One float on the zone row is cheap.
- **What moves it.**
  - Strikes (below).
  - Slow recovery toward 50 at 1 per 600 s when nothing happens.
- **What it drives.**
  1. **Informants (D9-legal intel).** Each director tick, a town with
     standing >= 50 where resistance units stand may yield a TIP with
     chance `standing / 200`: a sighting (count and rough position, +-100 m)
     in the sighting table, as if a local had talked. A bombed town stops
     talking.
  2. **Garrison morale.** Occupier garrisons spawned in a town with standing
     < 25 get skill x 0.8 (`SpawnGarrison`'s skill parameter).
  3. **Occupier income.** The passive refill is scaled by mean standing
     (5.2), exactly 1 at the default of 50.

**Effect of a strike (D21: "people hate war")**, live or abstract:

| Where | Resistance support | Occupier standing | Other |
|---|---|---|---|
| The struck town | -40 | -40 | heat +15; the town panics |
| Every town within 2 km | -10 | -10 | fear spreads; no panic |

Both values floor at 0.

**A struck held town is easy to retake.** A struck town held by the
resistance usually drops below `townRevertSupport` (30), so the next enemy
standing in it reverts it at once (D5, D18). That is the occupier's motive
for a strike, and the falling standing is its price.

**Panic, written natively.** Native code has no writer into the script
ledger `GM_PANIC_ZONES`, so a new zone event `townStruck` lets
`civilians.sqs` call `GM_fnPanicStart`. Because a panic episode (330 s) is
shorter than the 600 s economy tick, the economy also reads a native
`struckUntil` stamp on the zone row (strike + 1200 s). Until then the town
pays half income and no manpower.

**Live and abstract.**
- **Live:** the pass is real, and whatever the aircraft's own sensors
  acquire is engaged, civilians included.
- **Abstract:** no bodies are killed; the civilian cost is the support and
  standing loss.

**When a town strike is bought** (phase 6):
- only while STRONG, read on the pool the ground operation leaves AND on
  the pool after the strike's own price (2026-10-16, round 3.5): the
  airframe must leave the band STRONG after both debits. The director then
  takes the cheaper attack helicopter when the jet would demote the band,
  and buys no strike when neither keeps STRONG (one INFO line, "no strike
  with the operation on Le Port: a strike would take the pool below STRONG
  (pool 40 of 60)", video 4's figures, when Abel's poolStart was 60; it is
  48 since the generated bases, 5.7). Video 4: an urgent retake of Le Port (a BMP column,
  the pool paying 22) took a pool of 62 to 40, still STRONG, and the 23-unit
  jet bought with it against 4 reported fighters took it to 17, THIN, for
  38 s. The `gmForcesForce ["strike", zone]` lever keeps the old
  affordability (the pool can pay);
- only on an URGENT held town with `knownFighters >= 3`, or as the opening
  of a base or HQ assault with at least one defender reported (2026-10-16);
- or, since 2026-10-16 (the repel ladder, 6.3), as the ESCALATION STRIKE:
  with an operation on a held town or base that repelled the director
  twice (`approachRepels`), with `knownFighters >= 3` (a base: 1), outside
  its strike window, urgent or not. It is the one strike that may leave the
  pool STRETCHED: the airframe must leave the band STRONG or STRETCHED
  after the ground debit and its own (`PlanStrike` stretchedOk), never
  THIN ("no strike with the operation on Arudy: a strike would take the
  pool below STRETCHED"). Line: "[op N] escalation strike: Arudy repelled 2
  time(s), the pool stays STRETCHED (pool 16 of 48)". The band it names is
  the one the rule tested, the POOL's band after both debits
  (`PoolBandAfter`), not the band on the strength (5.6), which counts the
  airframe just bought as committed and so reads higher. Round 5: video 6
  wrote "the pool stays STRONG (pool 16 of 48)" at 5345 s, the strength
  (16 plus the 32 units of airframe committed) STRONG while the pool, 16 of
  48 (33.3 %, over THIN's 33 % entry), was STRETCHED. This answers the
  owner's tuning question 1 the way the feel assessment proposes: no price
  change, a strike after ground has failed.

**The opener is a strike on defenders.** With nobody reported there is
nothing to strike, so a base retake or HQ assault on a target whose
sighting row is empty (or decayed) buys no opener, and the town rule above
already keeps a standalone strike off a town with fewer than 3 reported.
The director's census of defenders is its sighting table (D9, 6.1), never
the true census. For the HQ the count is that of the zone it stands in;
an HQ outside every zone counts the reported positions within 150 m of
`HqPos`. The comparison video (tmp/of-director-video-3, both branches)
showed the old rule buying a 23-unit jet for an empty base with 0 fighters
reported. The `gmForcesForce ["strike", zone]` test lever still bypasses
the rule.

**One ledger movement.** The ground operation and its opener are bought
under one `BandHold` (as the air re-plan, 197a7d3): the band is not
stepped while the hold is open, so an opener bought and refunded in the
same step (a refused launch) shows no band edge. In the running game the
hold nests inside the frame hold (5.6, "When the band is stepped"), so the
launch's net movement is stepped once at the next frame, with the rest of
the director's tick; without the frame hold (a bare ledger) it is stepped
when the hold closes. A strike that flies and comes home later is a real
movement: its refund is capped at the ceiling (5.5) like any other.

It never targets a hidden player position (D9).

The reserve floor of one ground launch (6.3, 2026-10-17) does not apply to
strikes, which keep the band rules above; the ground operation a strike
goes with is floored on its own price, and the strike is then tested on the
pool that launch leaves.

### 7.4 The win

`CampaignEnd::AllHeld` requires every roster zone held at once, checked every
second; a recaptured base makes it false. Once `_won` latches,
OccupierForces and CounterAttack freeze: nothing launches, nothing in
flight steps, nothing reverts. Decision R is replaced.

## 8. Alarms and town presence

### 8.1 Presence rules (D5, D11, D23, D24)

**Occupier presence:**
- in a town: any living occupier unit within 150 m, plus abstract holding
  forces while the town is out of the bubble;
- in a base: the same, under the mirrored meter (`occCapture`);
- at the HQ: the HQ meter counts only HQ_ASSAULT units within 150 m of
  `HqPos` (9.1).

**No zone exclusion for the HQ (D24).** The zone the HQ stands in, held or
not, keeps its ordinary census and defenders. `450327e`'s own-target census
is reverted in phase 1 with no exception.

**Resistance presence for the player** comes as two predicates, both
reusing today's inputs (`SamplePlayerPresence`).

**HOLD** (blocks the drain of a held town and the occupier's base meter):
- **Not undercover**, or wearing a **warrior (uniform) outfit**: he always
  holds, weapon out or not (D23). `gmUndercover` is true for the whole
  campaign (`undercover.sqs`), so it is the outfit clause that does the
  work.
- **Disguised** (civilian outfit): he holds only when ANY of these holds:
  - **exposed**: the global `LivingWitnessCount() > 0`;
  - **weapon in hands**;
  - **weapon seen**: an occupier observer's current per-observer verdict
    from `EvaluateUndercoverRule` is suspect-by-weapon. That verdict
    already covers a slung weapon seen from behind (`undercoverBackArcCos`,
    `undercoverSlungBoost`) and at conversational range
    (`undercoverNoticeRadius`).

**CAPTURE** (raises a town's support or the player's own capture meter):
#86's rule, which narrows capture only. He counts when exposed or with a
weapon in hands; outfit alone does not count.

**`guerrilla_town_defenders`** is updated in phase 1:
- t5 (the armed player alone takes the cleared town) draws a weapon and
  asserts the weapon-in-hands slot instead of the outfit slot;
- a new case: a uniformed player with his rifle slung stands in a held town
  and holds it against a drain.

### 8.2 Alarm answers (D4)

A RED occupier zone is an alarm. The answer is an ALARM record, a new kind,
because `LaunchFrom` takes only towns:
- **Source.** The nearest OTHER occupier base by travel time, the airfield,
  or off-map air, never the zone under attack.
- **Package.** As in 6.5: minimum 4, always urgent, the arrival rule
  (6.5) applied outside THIN. Lethality is set against the SIGHTINGS for
  that zone, not the alert `knows`.
- **Movement.** Born under 3.1, moves at real speed, commits to its
  destination.
- **End.** At arrival it searches the last-known positions. It ends when
  the zone is below RED and it found no contact. Survivors join the
  garrison up to its room, as a live top-up does (5.5); the rest walk
  home as stragglers.
- **Limits.** One alarm answer at a time. THIN answers only with the
  cheapest package; SPENT does not answer.
- **The reserve floor** (6.3, 2026-10-17). An answer to an occupier-held
  TOWN may not take the pool under `forcesPoolFloor` x poolStart; it waits
  (one line, no rest set) and the next RED zone is weighed in the same
  step, so a base alarm behind it is still answered. An answer to a BASE
  under attack is exempt and may dig into the reserve.

Until phase 6, and on old saves, the free script QRF (`qrf.sqs`) runs as
today.

## 9. HQ loss, win and lose

### 9.1 The HQ (D13, D14, D22, D24)

**One HQ model.** `GuerrillaBase` is the only source of truth for where the
HQ is. Two changes make it so:
- **The default start is a real HQ.** With no START TOWN picked (the
  "(camp)" default, a direct launch or a failed pick), the first active
  tick of a campaign (`campaignEnd` on) ESTABLISHES the HQ at the first
  CAMP zone through `Establish`, tried once per session and never after a
  destruction (`TryAutoElection`, `WantsDefaultHq`); `campaignEnd = 0`
  fixtures keep no HQ at boot. Old saves with no HQ get the same on load,
  unless the HQ was destroyed.
- **The start-town exemption.** A START TOWN HQ keeps the #79 B2 garrison
  exemption (tied to the start-town pick, not to the HQ); a re-established
  HQ never gets it (an accepted deviation).
- **A saved `_hqDestroyed` flag** separates "destroyed" from "never
  elected". New accessors:
  - `HasHq()`: established and not destroyed;
  - `HqPos()` (exists);
  - `HqZone()`: the zone containing `HqPos`, else -1, for logs, cluster
    scoring and the phase-1 dispatch skip.

  A retaken Camp does NOT become the HQ again: only an establish action
  makes an HQ.

**The recruit anchor follows the HQ.** `recruit.sqs` reads `gmHqPos`
instead of the first CAMP zone. While there is no HQ there is no recruiting.

**The HQ meter (D24).** The HQ falls through its own meter, tied to `HqPos`
and independent of zone ownership:
- `hqCapture` (0..100), saved in the `GuerrillaBase` block.
- **Rising.** It rises by `captureRate` x min(assault units, 3) per 3 s
  zone tick while units of CAKHqAssault records, live or abstract, are
  within 150 m of `HqPos` (`CounterAttack::HqAssaultPresence`) and no
  resistance HOLD presence (8.1) is there. That counts the player under the
  hold rule and every live resistance unit, his recruits and companions
  included.
- **Frozen** by resistance HOLD presence within 150 m while assault units
  are there.
- **Decay.** With no assault units it decays at `captureDecayAbandoned`
  (2). There is no defended-decay path (`GuerrillaBase::NextHqCapture`).
- **Who counts.** Only HQ_ASSAULT records, so passing patrols, columns and
  QRFs never touch it (D14), a patrol parked on the HQ included. HQ
  records settle on `hqLost` (5.5).
- **Where it works.** The same everywhere: at the Camp, in a held town, in
  an unheld start town, or in an enemy zone (D24).
- **The zone around it is unaffected.** The HQ never suppresses that zone's
  defenders or census, and the zone follows its own ordinary rules: a
  patrol may revert the town the HQ stands in, while the HQ itself stands.

**When the meter reaches 100, the HQ is DESTROYED** (a new
`GuerrillaBase::Destroy()`):

| Asset | What happens |
|---|---|
| Weapon cache | Emptied at once (no looting the ruins); the holder and the prop are queued as `Ruins` (saved) and deleted once the perception gate rates them hidden (`SweepRuins`). |
| Garage (D22) | Every vehicle locked in the garage is DESTROYED in place: unlocked, damage 1, wrecks left where they stood. The ring is removed. |
| Recruit point | Gone with the HQ (it follows `HqPos`). |
| The zone around it | Unchanged: it keeps its owner and its ordinary rules (D24). |
| State | `_hqDestroyed` set, `_established` cleared. A new `_hqEpoch` counter is bumped, so scripts can tell this HQ from the next. |

**Re-establishing (D24).** Anywhere the existing flow allows: the "Establish
headquarters here" action (`gmHqEstablish`) inside any zone, held or not.
- It is a first establishment, so it is free: the `hqMoveCost` debit
  applies only to moving an established HQ.
- The cache starts empty and the garage ring is fresh.
- `_hqDestroyed` clears.

**`market.sqs`.** This is existing UI, not new messaging. It watches
`_hqEpoch`:
- it deletes the "HQ (zone)" marker while there is no HQ;
- it re-arms `gmMktHqSeen` and the `hqEstablish` objective;
- it places the new marker on re-establishing.

The fall also writes an INFO line (D12) and fires a new `hqLost` zone event
(serialized slot). Player-facing messaging is deferred. Respawn at the HQ
is out of scope.

**The phase-1 minimal piece (N4a).** A minimal `HqZone()` lands in phase 1,
and the legacy CounterAttack dispatch (`Decide`, `LaunchFrom`,
`OnTownFlipped`) skips the HQ's zone as a target. The HQ town is thus never
the object of a flip answer before the director exists. Passers-by can
still drain that town (D5), but no HQ_ASSAULT exists before phase 4, so the
HQ itself is safe until then.

### 9.2 Lose (D10, D13)

**Detection.** `CampaignEnd` gets a `Lost` latch, evaluated each second
after the win check, never once won, and absent with `campaignEnd = 0`.
The latch is stored only there. The campaign is LOST when ALL of these hold:
- the player has NO HQ (`!HasHq()`: destroyed and not re-established);
- the resistance owns no zone of any type (`HoldsAnything`: towns, posts
  and the Camp alike; the HQ's own zone counts only if the resistance owns
  it);
- the campaign "had something to lose": an HQ was ever established
  (`HqEver`: established, destroyed or epoch > 0) or a zone was ever held
  (`OnceHeld`, saved). A campaign that never had either is not lost.

**The opening is safe.** The HQ falls only to a deliberate, AWAKE-only
assault (D14), so a patrol can never end the game in the first minutes.

**What this means with D24.** The player may re-establish anywhere, for
free, so losing the HQ while he still holds something is a setback.

He is beaten in either of two cases:
- **Lost with nothing held.** The HQ is destroyed while he holds no base
  and no town. The latch is checked every second, and he cannot have a
  second HQ ready.
- **Lost after the HQ.** He loses his last base or town while he has not
  re-established an HQ.

**End path** (code change):
1. `OnLost` writes an INFO line.
2. After `campaignEndDelay`, `RequestEnd` sets `EMLoser` once, while the
   mode is still `EMContinue`.
3. `UIMapDialogs.cpp` gains an `EMLoser` branch that reads
   `CampaignEnd::DebriefingText`. It returns the lose title and body when
   `Lost()` (for example "Malden is lost"; days survived; bases and towns
   once held). No `EMEnd2` fallback: `EMEnd2` would play `OutroWin` and
   count as a completion.
4. The route is `exit.sqs`, `DestroyMap`, the defeat debriefing,
   `OutroLoose`, then the main menu.
5. Saves stay on disk, and GUERRILLA > Continue loads the latest. A save
   taken while doomed reloads into the same loss; that is accepted.
6. OccupierForces and CounterAttack freeze on the latch.

**Test.** `remaster_ud_guerrilla_forces_lose` (under
`tests/integration/ui/remaster`) follows `remaster_ud_guerrilla_abel_win_screen`:
- no `--autotest` on the end leg;
- `gmCampaignEndDelay` holds the window;
- it asserts `triDisplay == 50` and the lose text on control 102.

## 10. Internal pacing reference (not player-facing)

| Moment (Abel, STRONG) | A force comes |
|---|---|
| Town 1 | 0.40 |
| Town 2 | about 1.0; the director wakes with 2 tokens |
| Town 1 at that moment | quiet 0.63 / loud 1.0 at 10 min; quiet 0.25 / loud 0.63 at 20 min |
| One flip per 15 min | about 1 answer per 2 flips |
| STRONG and nothing hot | base retakes (D16), HQ assaults and push-out about every 30 min |
| THIN / SPENT | 1 answer per 60 min / none until 10 % refilled |
| Exhaustion to full | worst case about 4 h (everything empty, D15; STRONG again at about 3.5 h); typical about 1.6 h (only the pool empty) |

**Arrival over 3.5 km of road** (estimates until read from config):

| Package | Arrival |
|---|---|
| Foot | about 39 min |
| Truck | about 4-5 min |
| Helicopter from the Airfield | about 2 min + landing |
| Helicopter from off-map | about 4-5 min, at 1.5x the price |

**Waves:**

| Wave | Typical trigger |
|---|---|
| 1 | Outpost + 5 towns, or Outpost + Compound + 1 town |
| 2 | Outpost + Compound + 10 towns, or three bases + 5 towns |
| Final stand | the third base falls |

## 11. Persistence

**`GuerrillaOccupierForces` block** (two-pass, gated on `HasState()`):
- pool, poolStart, R, K, G, the roster and the excluded towns;
- `_townsEver` (a name set), posture, tokens and their clock;
- the urgency latches;
- the director accumulator and the op counter;
- the refill accumulator;
- the band (for hysteresis);
- the wave state and surges;
- cooldowns;
- the sightings (zone, count, positions, stamp);
- `lost` diagnostics;
- RED payments, the op ledger and `legacySave`. A test lever's funded
  operation (`gmCounterAttackLaunchFunded`) books a `lever` row: it settles
  refunds and losses and drives no director action (L14). A row leaves the
  ledger only when it is finished with nothing outstanding; past four times
  `MaxOpRows` the oldest finished row is settled to the pool first (L6).

**Elsewhere:**
- On `ZoneSaveState`: pending deposits, `occCapture`, `standing` (-1 =
  absent in an old save), `struckUntil` (nothing writes it until the phase 6
  strikes) and `lostCount`.
- In `GuerrillaCampaignEnd` only: the win latch, the lose latch (`lost`,
  `lostDay`, `lostMinute`) and `OnceHeld`.
- In the `GuerrillaBase` block: `hqDestroyed`, `hqEpoch`, `hqCapture` and
  the queued `Ruins` (absent in older saves = not destroyed, epoch 0,
  meter 0).
- New zone-event handler slots `baseLost` and `hqLost`.
- CounterAttack `Record` gains `kind`, `origin`, `package`, `funding`,
  `priceUnits`, `destPos`, `hqDone`, `withdrawing` (a top-up squad walking
  home) and hull refs (second-pass resolution), plus the escort fields
  (class, crew, price, hull and crew-group links). The block also saves
  the `Leftovers` list. Hulls ride the world's vehicle serializer.
- `OccupierAir` records are transient in the spike (not saved); phase 6
  decides whether they persist.

**Old saves (D20).**
- The pool derives from config scaled by the current C.
- Crossed thresholds are marked fired.
- `_townsEver` is seeded from the towns held. So is a block saved before
  format 6 that loaded the set empty (phase 2 never filled it), on the
  second load pass with the restored zone owners; invariant W is
  re-stepped (L8).
- The sightings start empty.
- **Tokens start at 0** (not full), so a migrated save does not launch two
  operations within a minute of loading.
- An unestablished old HQ is established at the Camp (9.1).
- **A `legacySave` flag** is saved in the block, true when the block was
  derived from an old save. While it is true the native alarm answers stay
  off, so they never run beside the old save's own script QRF.
- **The core behaviour applies natively.** The old free QRF and the fast
  heat decay come from the script text the save carries (#79 P5), so they
  stay until a new campaign with no flag needed for them.

**Determinism.** Rolls are hashed, with no engine RNG. A reload reproduces
pool, posture, tokens and wave timing. Package choice is reproducible to
within one heat-decay step.

## 12. Testing

### 12.1 Unit (`test_occupier_forces.cpp` `[occupierforces]`, plus existing suites)

- **Pool.**
  - K over every administered non-exempt CITY and the exemption; an
    empty roster; `campaignEnd = 0`.
  - The settlement probe (5.1): a seeded label the probe rejects is not
    counted (the table path and the stored-verdict path); the verdict is
    probed once and stored (a later probe that answers otherwise changes
    nothing); the probed-out count survives the save; a save without the
    block probes when it derives; an unprobed seeded town counts. In
    `test_zone_registry.cpp`: `ProbeSeededSettlements` probes seeded rows
    only, once, and the row's verdict survives save and load.
  - The refill formula: the 4 h figure from full exhaustion with D2
    drawing first.
  - Band hysteresis: no SPENT/THIN flapping, including the recovery phase
    (D2 running in SPENT, the pool near 0 while garrisons are short, one
    exit at 10 %).
  - The best-case 1.6 h and worst-case 4 h refill.
  - The standing factor is 1 at 50.
  - Draw and the floor.
- **Conservation.** Every path in 5.5: passer-by reverts with a pool
  refill, a travelling record turning into a top-up column, hull refund /
  loss / re-create, cancelled unborn operations, waves.
- **U1 to U5.**
- **W.** Base first, then town 1: one roll. Town 2: AWAKE.
- **Candidates.**
  - Towns.
  - Bases only when STRONG; quiet bases through push-out.
  - The held-zone decay applies to bases.
  - The HQ only when STRONG, in a wave, or as the final push.
  - Urgency hysteresis: a fresh quiet flip is never urgent; an edge latches
    urgency until heat < 35.
- **Anti-1:1.** Four flips 15 min apart give fewer than 4 operations.
- **Push-out.** Only when STRONG.
- **Packages.** Ladder classification for WEST, EAST and LoBo Hizballah;
  overrides. The choice table and both worked examples, crew included. No
  war-level gate. The group and hull budgets.
- **Resolution.** With forces on, every arrival against present defenders
  goes live (towns and bases), and over budget it waits at the edge doing
  nothing. Empty targets: hold and drain. Key off: the legacy `Fight` with
  `A_lethal` and `A_bodies` kept apart.
- **Intel.** Choices are unchanged when the true census changes but
  sightings do not. A dead group's last report is kept. The player-position
  fallback is never read.
- **Birth chain.** Launch, backward, next base back with re-sourcing and a
  refund, cancel unseen (phases 3-5), the air re-plan (phase 6), unborn
  operations not counted, the air gate at altitude, off-map admissibility.
- **Base recapture.**
  - `occCapture` is separate from `capture`: no "Securing" ticker, no
    `ZECaptureStarted`.
  - A hold squad freezes it, and it decays when abandoned.
  - Abstract BASE records tick it off-screen; the remote tick.
  - `baseLost`.
  - Re-capture effects: no duplicate hold squad (per-zone tracking),
    "Retaken".
- **HQ.**
  - The default camp start establishes a real HQ.
  - `HasHq` / `HqZone` / `_hqDestroyed`.
  - The HQ meter at `HqPos`: assault units only, passers-by ignored, works
    in an unheld start town and in an enemy zone. The zone's census is
    unchanged by the HQ.
  - `Destroy`: the cache emptied, garage vehicles destroyed, the epoch
    bumped, the recruit anchor gone.
  - A retaken Camp is not an HQ again.
  - Re-establish anywhere, free.
  - The phase-1 dispatch skip of `HqZone()`.
- **Lose.**
  - No HQ, no base, no town: `EMLoser` once.
  - The HQ destroyed while holding nothing: lost. While holding something:
    not lost.
  - Never after a win.
- **Presence.**
  - HOLD: uniformed counts weapon or not; disguised counts only exposed, in
    hands, or with a suspect-by-weapon verdict (slung from behind, notice
    radius).
  - CAPTURE (#86): exposed or in hands only.
- **Town strike and standing.**
  - Struck town -40 / -40, neighbours within 2 km -10 / -10, floors at 0.
  - `townStruck` event and `struckUntil` stamp.
  - Informant tips at `standing / 200` only when >= 50.
  - Morale skill below 25.
  - Never a hidden position.
- **Serialization and old-save derivation.**

### 12.2 Trident (`packages/RemasterUD`, `Guerrilla.Abel`)

Every birth and pop-in test first asserts a real camera (`gmTrafficPercept`
`hasCamera`, the 750 m day floor), pins daytime and the default view
distance, and is not `headless`.

| Test | Proves |
|---|---|
| existing `counter_attack_*` | Unchanged with the key off (legacy speeds); conservation changes asserted with it on |
| existing `guerrilla_town_defenders` (edited in phase 1) | t5 captures with a weapon in hands (asserts that slot, not the outfit); a new case: a uniformed player with a slung rifle holds a town against a drain (D23) |
| `guerrilla_forces_watched_route` | Whole Harbour route watched: the landmasses (the Southeast Compound on its own island), no road route from the Compound, and the re-source to the Airfield (updated in `bcca80e`; before it the test re-sourced to the Compound and its squad stalled inside the west gate) |
| `guerrilla_forces_passerby_revert` | A parked patrol reverts an empty town; the refill is pending and pool-paid; the patrol leaves |
| `guerrilla_forces_no_popin`, `guerrilla_forces_wiped_release` | S5 |
| `guerrilla_forces_both_towns` | Two ops; INFO lines joined to `gmOccupierOps` ids |
| `guerrilla_forces_motorized` | Urgent far target: a truck squad drives and dismounts at 250 m |
| `guerrilla_forces_base_retake` | STRONG pool: the director retakes a base; a hold squad forces a live fight |
| `guerrilla_forces_hq_assault` | Default camp start has a real HQ. A patrol through the Camp does nothing to the HQ. An assault fills the HQ meter; the HQ is destroyed: cache emptied, garage vehicles wrecked, marker removed. The player re-establishes in an enemy town; that town's garrison still defends it (D24). |
| `ui/remaster/remaster_ud_guerrilla_forces_lose` | No HQ, base or town: the defeat debriefing (the win_screen pattern) |
| `guerrilla_forces_refill_pushout` | `gmForcesForce ["refillHours", 0.05]`: refill, then push-out |
| `guerrilla_forces_wave_base` | Wave 1 strikes the densest cluster and retakes a base; no win while it is enemy-held |
| `guerrilla_forces_armoured_column` | Phase 5 (landed): a carrier and its escort born unseen under the headlight gate, kept together (150 m / 60 m), the escort supporting at the edge |
| `guerrilla_forces_heli_insertion` | Air spike (landed as a spike test, `c5cfa44`), tag `slow_air`, retries 3, not in the default `-j` run: off-map airborne birth gated at altitude on an explicit birth point; helipad LZ; two groups; unload; crew deleted before the hull |
| `guerrilla_forces_jet_sortie` | Air spike (landed as a spike test), `slow_air`: an airborne jet birth, the search orbit, the limited reveal, removal crew first |
| `guerrilla_forces_air_op` | Phase 6 (landed, `bcca80e`), `slow_air`: a RETAKE bought as a helicopter insertion from the Airfield through `CounterAttack`; the Mi17 born airborne beyond perception, the LZ 300..500 m out, a landing within 100 m of it, the cargo the live squad, the airframe removed unseen crew first |
| `guerrilla_forces_air_save_reload.seq` | Phase 6 (landed, `bcca80e`), `slow_air`: an insertion saved mid-flight (`01_inflight`) resumes in a fresh process (`02_resume`) with its air fields and the OccupierAir hull, crew, cargo and pad refs |
| `guerrilla_forces_town_strike` | Phase 6 (landed, `bcca80e`), `slow_air`: a strike on a held town; -40/-40 there, -10/-10 within 2 km, nothing beyond, `townStruck` panic, D18 (no passive recovery), half income until `struckUntil` |
| `guerrilla_forces_alarm_answer` | Phase 6 (landed, `bcca80e`): a RED occupier zone answered by one ALARM operation from another base while DORMANT, with `qrf.sqs` silent; `legacySave` brings the script QRF back |
| `guerrilla_forces_save_reload.seq` | Pool, bands, posture, tokens, waves, ops and hull refs across a fresh process |

### 12.3 Human session

- Pacing.
- Watched arrivals.
- Helicopter landings and frame time.
- Whether HQ loss reads as a setback and the loss as a defeat.
- Air strikes on towns.

## 13. Risks (implementation-facing)

| Risk | Mitigation |
|---|---|
| AI helicopter landing may drift off the LZ | Helipad, landing validation, a failure re-plan, the phase-6 spike first; ground packages never depend on air |
| Passers-by erode towns (D18) and take bases (D19) | Accepted by the owner; visible in the INFO log |
| Hulls and two-group insertions press the budgets | Per-package group counting, `forcesLiveHullsMax`, launches wait |
| Base recapture can stall the win | Finite waves; the refill caps at poolStart; the win latches the moment all bases are held |
| Old saves get the core only | D20 |

## 14. Implementation plan

**CounterAttack additions** (beyond `4400fe9` and `450327e`):
1. Record kinds RETAKE, BASE, HQ_ASSAULT, ALARM, REINFORCE, STRIKE.
2. `Schedule` with a fixed source.
3. `LaunchFunded`.
4. The AWAKE hook.
5. Packages: hull spawn, the driver idiom, dismount, refunds, deletion on
   return to abstract travel.
6. Air: native airborne creation, helipad LZ, two groups, `IssueCommand`
   pilots, crew-first deletion, strikes with limited reveal.
7. The birth chain: re-source to the next base back, then the air re-plan,
   then cancel unseen.
8. Real speeds (key on only).
9. `KeepFloor`.
10. `SettleRevert`, travelling records as top-up columns.
11. Base recapture and HQ records.
12. `Fight` with lethality.
13. The `Won` / `Lost` freeze.
14. `Record` fields.

**Phases:**

| Phase | Content |
|---|---|
| 0 (landed) | `4400fe9`, `450327e` |
| 1 | Unconditional: U1-U5 (with U5's key-off refund going to the source zone or discarded); the census reverted to "any enemy" (no HQ exception, D24); the HOLD and CAPTURE predicates (8.1, D23, #86) with the `guerrilla_town_defenders` edit; a minimal `GuerrillaBase::HqZone()` with the legacy dispatch skipping the HQ's zone (N4a); a statement that these change every mission |
| 2 | `OccupierForces` core: roster, K, pool, the refill formula, banded hysteresis, control, war-level split, persistence, `gmOccupierForces`, `gmForcesTune` / `gmForcesForce`; pool routing (inert, key off) |
| 3 | Director: sightings at 5 s (`gmOccupierIntel`), W, candidates with urgency, tokens, push-out, foot and motorized packages with ladder classification, real speeds, the birth chain up to cancel-unseen, INFO lines, `gmOccupierOps`. Abel opts in. |
| 4 | Waves; base recapture (`occCapture`, remote tick, `abstractOcc` for BASE records, `baseLost`, the `capture.sqs` audit with per-zone hold groups); D16 base candidates and bases in push-out. The HQ model (9.1): the default camp HQ, `HasHq` / `_hqDestroyed` / `_hqEpoch`, the HQ meter at `HqPos`, `Destroy` (cache, garage D22), the recruit anchor on `gmHqPos`, the `market.sqs` marker re-arm, `hqLost`. The lose latch and the `EMLoser` debriefing branch. Occupier `standing` with informant tips and morale (D21). |
| 5 | REINFORCE; armour packages (armed car, APC, tank) |
| 6 | Air: descriptor keys and library entries, the airborne-birth / landing spike, then insertions, strikes, town strikes (D17, D21: `townStruck`, `struckUntil`, the bought-when rule), the off-map edge, the air re-plan in the birth chain; alarm answers replace the script QRF |
| 7 | Defaults on (`campaignEnd = 0` missions pin it off). OFFSCREEN-CONTRACT rewrite: C, R replaced, S and S5, the HQ and lose rules, presence, budgets. Fork issue + `roadmap.yaml`. The HQ-respawn issue is separate. Messaging is a later item. |

## 15. (reserved)

## 16. Internal knob reference (implementers only)

**The levers** (phase 3 of round 4, owner decision 2; brief:
`tmp/of-director-levers-audit.md`; code: `OccupierForcesLevers.{hpp,cpp}`).
An author tunes the director through eight `CfgGuerrillaZones` levers.
`forcesDifficulty` (0 Easy, 1 Normal, 2 Hard, 3 Brutal; default 1) picks the
preset value of every lever; an authored lever key replaces its preset with
its own value, repaired into its range and logged. Normal derives exactly
the defaults from before the levers, field by field (the unit case
"levers: Normal derives today's defaults"), so a template that authors no
lever plays as before. The shipped templates author only `occupierForces`.

Every old key of the second table still reads, as an **expert override**:
the levers derive their fields first, then an authored old key replaces
the one field it names (its lever keeps deriving the others) and logs one
WARN per key and load (`<owner>: <key> is an expert override of the
<lever> lever`, or `... of a constant`). `gmForcesTune` keeps every field
name it had and adds the levers by the names in brackets: a lever
re-derives every field it folds, CounterAttack's three included,
overwriting earlier field values; `strength` takes effect at the next
activation (the pool is sized once) and ZoneRegistry's `heatCapSpike` at
mission load only. Tuning is not saved: a load re-reads the mission config.

| Lever ("gmForcesTune name") | Unit, range | Easy / Normal / Hard / Brutal | Folds (the Normal value) |
|---|---|---|---|
| `forcesDifficulty` ("difficulty") | preset, 0..3 | 0 / 1 / 2 / 3 | every lever below at that preset |
| `forcesCadence` ("cadence") | c, seconds per operation while STRONG, 60..7200 | 1200 / 600 / 420 / 300 | `forcesTokenInterval[]` = {c, 1.75c, 3c, 0} ({600, 1050, 1800, 0}); `forcesTownCooldown` = 3c (1800 s) |
| `forcesStrength` ("strength") | s, poolStart scale, 0.25..4 | 0.75 / 1 / 1.25 / 1.5 | poolStart = max(`forcesPoolMin`, round(s x (R + `forcesPoolPerTown` x K))) over the authored roster R, less the generated garrisons (5.5, 5.7); the shipped Abel (authored R 32 + 2 x 14 = 60, less its 12 generated) reads 33 / 48 / 63 / 78. Strike prices do not scale with it (NEXT question 1 stays open) |
| `forcesRecovery` ("recovery") | hours from exhausted to full, 0.5..48 | 8 / 4 / 3 / 2 | `forcesPoolRefillHours` (4, D15) |
| `forcesReach` ("reach") | r, seconds of ETA before it rides or flies, 0..86400 (0 = the arrival rule off) | 3000 / 1500 / 1200 / 900 | `forcesAirEta` = r (1500 s); `forcesSlowEta` = 0.6r (900 s) |
| `forcesAggression` ("aggression") | a, 0.25..4 | 0.75 / 1 / 1.25 / 1.5 | `forcesHeatFull` = `forcesRearmHeat` = 40/a (40); `forcesUrgentHeat` = 50/a (50) with its exit at 0.7 x (35); `forcesLethalityMargin` = 1.2a (1.2: with `counterAttackDefenderWeight` 3, L_req = 3.6a per fighter); the town strike's reported fighters round(3/a) (3; 4 / 3 / 2 / 2); `counterAttackFirstTownChance` = 0.4a (0.4); ZoneRegistry `heatCapSpike` = 40/a (40) |
| `forcesWaves` ("waves") | level, 0..3 | 0 / 1 / 2 / 3 | `forcesWaveThresholds[]` {0.70, 0.45} / {0.75, 0.50} / {0.80, 0.60} / {0.85, 0.65}; `forcesWaveShare` 0.35 / 0.5 / 0.6 / 0.75; `forcesWaveLead` 900 / 600 / 450 / 300 s; `forcesWaveRetakeShare` 0.4 / 0.5 / 0.6 / 0.7; `forcesWaveWarLevel` 0.75 / 1 / 1.25 / 1.5. Every level keeps two thresholds and the final stand |
| `forcesMinimums` ("minimums") | m bodies, 1..10 | 3 / 4 / 6 / 8 | `forcesRetakeMin` = the alarm minimum = m (4); `forcesHqAssaultMin` = `forcesSpareCapFloor` = m + 2 (6); `forcesSpareCap` on |
| `forcesConcurrency` ("concurrency") | n born operations in flight while STRONG, 2..6 | 2 / 3 / 4 / 5 | `forcesOpsMaxStrong` = n (3); `forcesOpsMax` = n - 1 (2); `forcesLiveHullsMax` = 2n (6); `counterAttackMaxLive` = n - 1 (2) |

*Deviations from the audit's proposal.* (1) Waves: the audit's Easy
{0.75} (one threshold) and Brutal {0.85, 0.65, 0.4} (three) would change
the number of big moments, against D7; every level keeps two thresholds
and the final stand and moves them instead (Easy later, Hard and Brutal
earlier). The audit gave no retake share or war level per level; the
levels step them by 0.1 and 0.25. (2) `forcesWaves` is a level, not a
quantity, and `forcesConcurrency` starts at 2 (n = 1 would set
`counterAttackMaxLive` to 0). (3) The audit's "one road factor" was
already landed by 16acfd5 (`forcesRoadFactor` 1.5 for the quote and the
march; `counterAttackRoadFactor` is retired: WARN, ignored), so it is a
constant here. (4) The two exposure options are both kept: the difficulty
key picks the presets, a lever key takes a value.

**Constants, author keys outside the levers, and test levers.** Kinds:
*lever* = a field a lever derives, its key an expert override; *constant
(key)* = a constant whose old key is still an expert override; *constant*
= no config key (a `gmForcesTune` or `gmCounterAttackTune` lever where
named, else an engine constant); *author* = an author key outside the
levers, no WARN; *legacy* = a `counterAttack*` key read as before, no
WARN. Owners: OF = `OccupierForces`, CA = `CounterAttack`, ZR =
`ZoneRegistry`, FP = `ForcePackages`, Script = `core/init.sqs`.

| Key / value ("tune name") | Owner | Default (range) | Kind | Meaning |
|---|---|---|---|---|
| `occupierForces` ("enabled") | OF, CA, ZR | 1 since phase 7 (`OccupierForcesTuning::KeyDefault`); `campaignEnd = 0` missions author 0 | author | the layer on or off |
| `forcesLog` ("log") | OF | 0 (0..1) | author (a diagnostic) | 1 logs the per-action lines of OF, CA and `OccupierAir` at INFO; 0 at DEBUG unless `--dev` (6.6); read with the layer off too |
| `forcesWaveHub` | OF | "" | author (geography) | the template's wave hub by zone name ("" = the rule, 7.1) |
| `forcesOffMapEdge` | CA | nearest edge | author (geography) | N / S / E / W: the parent nation's side; the point lies 1500 m beyond the map, level with the target |
| `forcesTruck` ... `forcesJet` | faction descriptor | ladder classification (air: none) | author (content) | hull class overrides; an air package is offered only when its class is authored |
| `forcesPoolMin` ("poolMin") | OF | 24 (0..200) | constant (key) | poolStart's floor, applied before the generated garrisons come off; dead on Malden (authored R + 2K = 60, then 48 after the 12 generated); read at activation |
| `forcesPoolPerTown` ("poolPerTown") | OF | 2 (0..8) | lever: Strength scales it | the K weight in poolStart; read at activation |
| settlement probe (`kMinBuildings` / `kRadius` / `kMinBuildingRadius`) | ZR | 3 / 300 m / 4 m | constant | a seeded CITY counts in K only on dry land with 3 buildings over 4 m within 300 m; probed once, stored on the zone row (5.1) |
| `forcesPoolRefillHours` ("refillHours") | OF | 4 (0.5..48 h; tune 0.01..48) | lever: Recovery | exhausted to full, garrisons included: (poolStart + R + G) / h an hour, x the standing factor |
| `forcesPoolFloor` ("poolFloor") | OF | 0.2 (0..1) | constant (key) | the THIN operations floor (`CanOperate`: no director operation while THIN under it), and since 2026-10-17 the reserve floor (6.3): a director-bought ground operation (director and push-out retakes of towns and bases, every ladder rung, the massing fallback, an alarm answer to a town) may not take the pool under it x poolStart (9.6 on Abel) by its own debit and waits instead; its plan is chosen on what the pool could pay at max(pool, poolStart) with it kept. Exempt: HQ assaults, wave operations, strikes, alarm answers to bases, the levers. 0 turns both off |
| band enter / leave | OF | STRONG 66 / 60, THIN 33 / 40, SPENT 0 / 10 % | constant | hysteresis on the strength (5.6), stepped once per frame on what the frame left (`StepBandFrame`; a wave landing and a forced pool at once) |
| `forcesDirectorInterval` ("directorInterval") | OF | 60 (10..600 s; tune 1..600) | constant (key), a test lever | the tick; tokens accrue on elapsed time, so it adds lag, not cadence |
| sighting cadence | OF | 5 s (`alertInterval`) | constant | intel sampling |
| `forcesIntelDecay` ("intelDecay") | OF | 1800 (1..86400 s) | constant (key) | a sighting's life; it also caps `forcesPushoutQuiet` |
| reveal limits | OF | sightings under 60 s, accuracy < 1 | constant | strike crews only |
| `forcesWakeTowns` ("wakeTowns") | OF | 2 (2..99; tune 1..99) | constant (key), a test lever (1) | distinct towns that wake the director |
| `forcesTokensMax` ("tokensMax") | OF | 2 (0..6) | constant (key) | the purse |
| `forcesTokenInterval[]` | OF | {600, 1050, 1800, 0} (0..86400 s per band) | lever: Cadence | seconds per token by band STRONG, STRETCHED, THIN, SPENT (0 = none); a short array keeps the cadence's values for the bands it omits; `LegacyTokenInterval` {1800, 2700, 3600, 0} is the old table (6.3) |
| `forcesTownCooldown` ("townCooldown") | OF | 1800 (0..86400 s) | lever: Cadence | the rest after a repel or a cancel, hashed +- `restJitter`; its end is the re-arm moment |
| `forcesOpsMax` ("opsMax") | OF | 2 (0..6) | lever: Concurrency | born operations in flight outside STRONG (`OccupierForces::OpsMax`; `CounterAttack::OpsCap` falls back to 2 while the service is inactive and adds one per living wave record) |
| `forcesOpsMaxStrong` ("opsMaxStrong") | OF | 3 (0..6) | lever: Concurrency | the born cap while STRONG is max(`forcesOpsMax`, this) |
| `forcesHeatFull` ("heatFull") | OF | 40 (1..100) | lever: Aggression | candidate chance = heat / heatFull per roll |
| `gmHeatDecayHeld` | Script | 0.25 per 10 s | constant (script) | held-town decay: the candidate window after a capture |
| `forcesUrgentHeat` ("urgentHeat") | OF | 50 (1..100) | lever: Aggression | urgency entry |
| urgency exit ("urgentExit") | OF (`urgentExit`; `UrgentExitHeat` 35) | 35 (0..100) | lever: Aggression (0.7 x the lever's entry); no key | urgency released below it; an authored `forcesUrgentHeat` leaves it where the lever put it |
| `forcesRearmHeat` ("rearmHeat") | OF | 40 (0..100; 0 = no re-arm) | lever: Aggression (= heatFull) | the re-arm (6.3): a rest ending on a held zone with fighters reported holds its heat here until an operation goes, its reports decay or it changes hands; saved names, `gmOccupierForces` [30] |
| `forcesPushoutQuiet` ("pushoutQuiet") | OF | 1800 (0..86400 s) | constant (key) | no sighting this long = quiet (towns and held bases); values above `forcesIntelDecay` are dead |
| `forcesRetakeMin` ("retakeMin") | OF | 4 (1..12) | lever: Minimums | a base retake takes min(max(this, garrisonMax), 12), lowered by the spare cap |
| alarm minimum ("alarmMin") | OF (`alarmMin`; `ForcePackages::AlarmMinBodies` 4) | 4 (1..12) | lever: Minimums; no key | an alarm answer's minimum bodies (8.2) |
| `forcesHqAssaultMin` ("hqAssaultMin") | OF | 6 (1..12) | lever: Minimums | minimum HQ assault bodies |
| `forcesSpareCap` / `forcesSpareCapFloor` ("spareCap" / "spareCapFloor") | OF | 1 / 6 (0/1 / 1..12) | lever: Minimums | the spare cap (7.2): a base retake or HQ assault no single base can source takes the largest single-base spare, never below the floor |
| `forcesLethalityMargin` ("lethalityMargin") | OF | 1.2 (1..3; tune 0..3) | lever: Aggression | L_req = margin x defenderWeight x fighters |
| `forcesUrgentCostShare` ("urgentCostShare") | OF | 1.5 (1..3) | constant (key) | an urgent package may cost this share of the cheapest |
| `forcesSlowEta` ("slowEta") | OF | 900 (0..86400 s; 0 = off) | lever: Reach | the arrival rule (6.5): past this ETA of the cheapest lethal package a faster one within the share may go; push-outs included |
| `forcesSlowCostShare` ("slowCostShare") | OF, FP (`SlowCostShare` 3) | 3.0 (1..5) | constant (key) | the share above |
| `forcesAirEta` ("airEta") | OF | 1500 (0..86400 s; 0 = off) | lever: Reach | past this ETA a transport helicopter insertion may go whatever its price |
| air fighters (`AirFightersMin`) | FP | 1 | constant | reported fighters that make a target "defended" for the air choice |
| `forcesRoadFactor` ("roadFactor") | OF | 1.5 (1..5) | constant (key) | the one road factor (16acfd5): a ground leg with no known road route, quoted and marched, is the straight distance x this |
| road quote cache (`RoadQuoteCache::DefaultTtl` / `DefaultPlansPerTick`) | OF (`RoadRoute.hpp`) | 600 s / 6 | constant | the road quote's entries and its plan budget per tick |
| town strike fighters ("strikeMinFighters") | OF (`strikeMinFighters`; `StrikeMinFighters` 3) | 3 (0..12) | lever: Aggression (round(3/a)); no key | reported fighters a town strike needs (7.3) |
| opener fighters, HQ radius (`StrikeOpenerMinFighters` / `OpenerHqRadius`) | OF | 1 / 150 m | constant | a base or HQ opener needs one defender reported; the radius around `HqPos` for an HQ outside every zone |
| strike band rule (`PlanStrike` keepStrong) | OF | STRONG after both debits | constant | a director-bought strike goes only on an airframe that leaves the band STRONG (7.3) |
| the repel ladder ("escalateLethality", "escalateInfantry", "approachRepels", "containRepels") | OF | 1.5 / 2 / 2 / 3 | constant | 6.3; kept constant by the lever round: they act only after repels, which no lever describes |
| massing, jitter, adjacency ("massingMax", "restJitter", "adjacentTown") | OF | 3 ticks / 0.2 / 1000 m | constant | 6.3; `adjacentTown` is a geography number, the jitter a feel number |
| value weights, salts | OF | 3 / 2 / 1 / 2; 7, 11, 13 | constant | candidate ranking (6.3) |
| `forcesControlMilWeight` ("controlMilWeight") | OF | 0.6 (0..1) | constant (key) | the roster's weight in control C |
| `forcesControlDwell` ("waveDwell") | OF | 2 ticks (1..10) | constant (key) | consecutive due ticks before a wave fires |
| `forcesWaveThresholds[]` | OF | {0.75, 0.50} (at most 3) | lever: Waves | control thresholds |
| `forcesWaveFinal` ("waveFinal") | OF | 1 | constant (key) | the final stand, on at every Waves level (D7) |
| `forcesWaveShare` ("waveShare") | OF | 0.5 (0..2) | lever: Waves | wave size, share of poolStart |
| `forcesWaveMin` / `forcesWaveMax` ("waveMin" / "waveMax") | OF | 12 / 48 | constant (key) | the wave size clamp; binds only for poolStart under 24 or over 96 |
| `forcesWaveLead` ("waveLead") | OF | 600 (0..7200 s) | lever: Waves | seconds inbound |
| `forcesWaveLeadFinal` ("waveLeadFinal") | OF | 120 (0..7200 s) | constant (key) | the final stand's lead |
| `forcesWaveRetakeShare` ("waveRetakeShare") | OF | 0.5 (0..1) | lever: Waves | the strikes' share of the wave's bodies, as staged infantry (hulls and crews paid on top, 7.1) |
| `forcesWaveOps` / `forcesWaveCluster` ("waveOps" / "waveCluster") | OF | 4 / 2000 m | constant (key) | strike targets and the cluster radius |
| `forcesWaveSurge` ("waveSurge") | OF | 12 (0..48) | constant (key) | final-stand surge above strength |
| `forcesWaveWarLevel` ("waveWarLevel") | OF | 1 (0..3) | lever: Waves | occupier war level per landed wave |
| wave surge ("surgeDecayHours", "surgeShare") | OF | 1 h / 0.5 | constant | the landed wave's surge above garrisonMax and its decay (7.1) |
| band loss window ("lossWindowHours") | OF | 1 h | constant | unrepaid losses count against the band this long (5.6) |
| `forcesBankHulls` ("bankHulls") | OF | 0 (0/1) | constant (key), a test lever | 0 = a hull over the ceiling comes back as a stored spare hull; 1 = it banks as bodies (the df9457e rule) |
| stored spare hulls (`StoredHullsPerBase`) | OF | 2 per base | constant | a base's garage for spare hulls (5.5); every hull refund, the direct air re-plan included, goes through `RefundHull` |
| pool ceiling (`PoolCeiling()`, wave headroom, `Banked()`, `Dropped()`) | OF | poolStart + headroom | constant | refunds never lift the pool above it; the excess surges, banks or is stored, and only what no base takes is dropped (5.5) |
| `heatCapSpike` | ZR | 40 | lever: Aggression (40/a, at mission load) | heat added on a capture |
| `forcesLiveHullsMax` ("forcesLiveHullsMax", gmCounterAttackTune) | CA | 6 (0..20) | lever: Concurrency | live hulls at once (read by CA, not `OccupierForcesTuning`) |
| `counterAttackMaxLive` ("maxLive", gmCounterAttackTune) | CA | 2 (0..8) | lever: Concurrency | live infantry records at once (more wait) |
| `counterAttackFirstTownChance` ("firstTownChance", gmCounterAttackTune) | CA | 0.4 (0..1) | lever: Aggression | DORMANT dispatch for the first town |
| `forcesSortieTime` | CA | 300 (60..1800 s) | constant (key) | a strike's time over its target |
| `counterAttack`, `counterAttackChance`, `counterAttackSize` / `WarLevelStep`, `counterAttackDelayMin` / `Max` | CA | 1, 1.0, 0.5 / 0.25, 120 / 300 s | legacy | the DORMANT dispatch and `ForceSize` |
| `counterAttackSourceKeep` ("sourceKeep") | CA | 0.25 | legacy | share of garrisonMax a base keeps home: every base's spare, so every director quote |
| `counterAttackSpeed` ("speed"), `MarchSpeed` | CA | 4 / 1.0 m/s | legacy, a test lever | key-off pacing; "speed" overrides the real speeds in tests |
| `counterAttackRoadFactor` | CA | retired | ignored (WARN) | `forcesRoadFactor` times every leg with no road route |
| `counterAttackDefenderWeight` / `counterAttackRepelLoss` | CA | 3 / 0.5 | legacy | one fighter = this many attackers (half of L_req); the key-off repel loss |
| `counterAttackLiveTimeout` ("liveTimeout") | CA | 1200 s | legacy | an unresolved live squad withdraws after it (out of sight) |
| `counterAttackBirthMinToTarget` / `counterAttackBirthGiveUp` ("birthMinToTarget" / "birthGiveUp") | CA | 250 m / 600 s | legacy (guards) | the birth bound and give-up (3.1) |
| break-off, hull wait (`BreakOffDivisor` / `WithdrawHullWait`) | CA | 3 / 120 s | constant | a live operation whose infantry x 3 is below its born strength withdraws; its hulls wait at most 120 s for it (3.2) |
| `EscortGap` / `EscortSpread` / `EscortRejoin`, dismount speed / `StopMax`, road-route join radius, hull gate, Leftover check | CA | 20 / 150 / 60 m; 1.5 m/s / 15 s; 2000 m; `NightEffect` 0.2, 2.5 m ray; 30 s | constant | columns, dismounts, routing and the hull gate (3.2) |
| cone-only base radius / audible floor | CA / Traffic | 300 m / 300 m | constant | the birth gate |
| prices: body, truck, armed car, APC, tank / transport heli, attack heli, jet / off-map | FP | 1, 3, 4, 8, 14 / 10, 18, 22 / x 1.5 | constant | pool units (6.5) |
| lethality: tiers / motorized / armed car / APC / tank / attack heli / jet | FP | 1.0-1.25-1.5 / 1.25 / 3 / 6 / 12 / 14 / 16 | constant | 6.5 |
| foot speed / ground cruise / air cruise | FP | 1.5 m/s / 0.6 / 0.8 of `maxSpeed` | constant | key on only |
| hull crew / infantry seats, columns, offered packages, town-edge role | OF, CA | crew = driver + gunner + commander (min 1), seats min(12, transportSoldier); a carrier + at most one escort; every ground package plus authored air | constant | 3.2, 6.5 |
| heli LZ / landing tolerance; `airAudible` / `landOrderDist` / `departTimeout` (gmAirTune) | OF, `OccupierAir` | 300-500 m / 100 m; 800 / 450 m / 900 s | constant | insertions and the airframe's flight |
| town strike effect | OF | struck town: support -40, standing -40, heat +15, panic, `struckUntil` +1200 s; towns within 2 km: -10 / -10 | constant | D17, D21 |
| occupier `standing` | OF (zone row; per-CITY `standing` key) | 50 (0..100) | author (per town) | recovers +1 per 600 s toward 50; tips, morale and the refill factor 0.75 + 0.5 x mean / 100 |
| `meanStanding` ("meanStanding") | OF | 50 | a test lever, no key | the refill factor's input |
| HQ meter | GuerrillaBase | `captureRate` x min(HQ_ASSAULT units, 3) per tick within 150 m of `HqPos`; frozen by HOLD presence | constant | D24 |
| `occCapture` | ZR | `captureRate` 6 x min(crew, `captureCrewCap` 3) per 3 s; defended -10, abandoned -2 | constant (zone keys) | the enemy base-retake meter |
| `baseRetake` | ZR | follows `occupierForces` unless authored | author | 0 = held posts are permanent |
| `budgetLiveBodies` / `budgetSideGroups` / `emptyGroupRelease` | ZR | 130 / 40 / 60 s | author (engine guards) | the live-AI and group budgets (OFFSCREEN-CONTRACT.md 6) |
| `campaignEndDelay` | CampaignEnd | 10 s | author | win and lose delay |
| `seedBases` | ZR (`BaseDetect`) | absent: 1 when the template authors no AIRFIELD and no SEAPORT, else 0 | author (geography) | runtime seed of the detected AIRFIELD / SEAPORT zones; ISLAND-PACKS.md section 10 |
| `playRegion[]` | ZR | absent: the authored zones' box + 5 km, for the base seed only | author (geography) | the theatre: base seeding, the CITY seed and the START TOWN list stay inside it (ISLAND-PACKS.md section 10) |
| seeded base garrison / income | `BaseDetectOptions` | AIRFIELD 12, SEAPORT 8 / the OUTPOST income | constant | plan 13 yields are not implemented |

## 17. Still open

Every owner question is answered (D1-D24). Open implementation items, as of
`bcca80e` and the phase 7 work:
- **A slow native alarm answer could contest a later Showcase capture
  chapter.** Chapter 6 and the human "alert" case accept a native ALARM
  operation as the answer; one still marching when a later chapter
  captures a zone could interfere.
- **The HQ-respawn issue** (separate, D13).
- **Player messaging** for the director, waves, strikes and the HQ (D12:
  gm* commands, and INFO lines only under `--dev` or `forcesLog = 1` since
  2026-10-14, 6.6). Journal integration is filed as GearUnclear/CWR#98, with
  the form left open.
- **#76 occupation network: what remains after auto-detection.** The
  runtime seed (`seedBases`, `playRegion[]`) and the offline detector
  (`guerrilla scaffold` writes detected AIRFIELD / SEAPORT / OUTPOST zones
  with evidence, `guerrilla lint --bases` audits a template) landed
  2026-10-14 (ISLAND-PACKS.md section 10). Still open: each template's
  **outpost ladder** is the author's (detected OUTPOSTs are suggestions,
  never seeded), the AIRFIELD / SEAPORT **yields of plan 13** (heavy-vehicle
  unlock, WL3 gate) are not implemented, so a base pays like an OUTPOST, and
  the START TOWN menu honours only an AUTHORED `playRegion[]` (the default
  box bounds the base seed alone).
- **Parked on the vehicle system: the Southeast Compound stays air-only.**
  The Compound is an island (3.1, `bcca80e`), so it sends only aircraft;
  boats are a later vehicle-system item, not an occupier-forces one (owner,
  2026-10-14: the vehicle system is not fully developed yet).

Closed 2026-10-14 (owner decision): **alarm answers beside the player.** An
alarm answer is born at its source base and travels at real speed even when
its target is beside the player, as built (3.1 step 1); no birth near the
target is added.

Closed after `509e30d` (#97): **the air re-plan's price no longer
bypasses the director's ledger** (3.1 step 6). `OccupierForces::SpendOnOp`
puts the airframe on the operation's row (package and outstanding), so its
`CAOpHullRefund` / `CAOpHullLost` settle through the row like a ground
hull's; `airReplanned` now marks only a re-plan no row could take (and
saves from before the change), which still settles straight to the pool.
The unit cases assert pool + outstanding across a re-plan that lands, one
refunded while abstract, and one shot down. The airframe's debit and the
ground hull's credit are one ledger movement: the band is stepped once on
the net pool (`OccupierForces::BandHold`), so a re-plan can no longer show
a false STRONG to STRETCHED edge between the two (found in the director
video run of 2026-10-15; `guerrilla_forces_replan_band` pins it).
Affordability stays gross: `Spend` still judges the helicopter price
against the pool before the hull returns.

Closed after `402ece6`: **K and the settlement probe** (5.1): the probe
returns for K. A seeded town counts only when it passes it, the verdict is
stored on the zone row once, and Sinai's K falls from 37 to 26.

Closed after `08c6ae2`: the live adoption cap (5.5, 8.2: a live top-up
or alarm answer joins up to the room, the surplus walks home); the LOW
review items L6, L8, L10, L11 (3.3, 11), L13 (a live force whose road ahead
passes the player stays live instead of returning to abstract travel and
being re-born behind him; a hull driving home to a base the resistance took
turns for the nearest occupier base on its landmass, else stops and waits
to leave unseen), L14 (lever ledger rows; NaN `gmForcesTune` /
`gmForcesForce` values take the low bound) and L15 (section 18). Declined:
L13's `ArriveLive` trap (it needs a source base within 251 m of its
target; the give-up path then cancels unseen and the holding path waits
at the edge by design), and L14's `gmForcesTune "enabled"` not surviving a
load (a test lever; the key is the mission config).

Closed since `c5cfa44`: the Southeast Compound gate stall (the Compound is
an island: an air-only source, resolved by the landmass rule, 3.1);
seated deletion in `Traffic.cpp` (`2524c3a`) and in `OccupierAir`
(`bcca80e`), both now moving a seated man out first; the corpse that could
vanish in view on a return to abstract travel or a top-up removal (both
now wait for the dead too, `BodiesUnseen`, `bcca80e`); the default flip
(phase 7A, status table).

## 18. Decision and critique log

| Round | Outcome |
|---|---|
| 1 | 4 BLOCKING, 11 MAJOR, 14 MINOR, fixed in round 2 |
| 2 | 2 BLOCKING, 7 MAJOR, 9 MINOR, fixed in round 3; S aligned with what landed as `450327e` |
| 3 | READY; touch-ups applied |
| 4 (owner interview) | D1-D12 applied: packages and lethality, air, real speeds, any-enemy drain, the D6 birth chain, waves on bases, refill and push-out, the intel limit, the lose rule, the presence rule, INFO-only messaging. Removed: the ownership filter, turn-back with refund, war-level purchase gates, fixed QRF timers, director journal lines, decision R. |
| 4 critique | READY: 12 MAJOR, 14 MINOR, 3 owner questions |
| 5 (owner answers) | D13-D20 applied: HQ destruction and re-establish, lose only with no HQ, base or town (9); HQ only to deliberate assaults (5.4, 8.1, 9.1); refill full in 4 h with garrisons (5.2) and band hysteresis (5.6); base retakes when strong (6.3); town strikes (7.3); towns stay weakened (5.4); any-enemy base recapture (7.2); plain-view landings and old saves (3.2, 11). |
| 5 critique items | **M1:** D14. **M2:** D18. **M3:** the `EMLoser` debriefing branch, no `EMEnd2`, the Continue reload, the test pattern (9.2). **M4:** lethality `Fight`, live base fights (6.5, 7.2). **M5:** airborne native birth, gate at altitude, off-map as sea, helipad, two groups, `IssueCommand`, crew-first delete (3.1, 3.2). **M6:** 5 s sightings, dead group's last report, limited reveal, alarm sizing from sightings (6.1, 8.2). **M7:** urgency with hysteresis (6.3). **M8:** cancel unseen, unborn not counted, re-sourcing, a phase-4 hub fallback (3.1, 7.1). **M9:** D19, remote tick, the `capture.sqs` audit (7.2). **M10:** D15 (5.2, 5.6). **M11:** ladder classification (6.5). **M12:** the undercover gate, `EvaluateUndercoverRule` reused (8.1). **MINOR 1:** CA keys, speeds key-on only (3.2, 16). **2:** km/h. **3:** crew priced in the examples. **4:** the ALARM kind, minimum 4, RED at a held town is a candidate (6.3, 8.2). **5:** a name set. **6:** one lose latch, none with `campaignEnd = 0`. **7:** travelling records become top-ups. **8:** hulls on return to abstract travel. **9:** group counting. **10:** HQ garrison ceiling, new kinds, GuerrillaBase assets. **11:** wave wording and keys (7.1). **12:** log prefix, rate limit, `gmOccupierIntel`. **13:** test levers, `slow_air`. **14:** hull budget. |
| 6 (owner answers + critique 5) | D21-D24 applied. **D21:** per-town occupier `standing` (informants, morale, refill factor), strikes -40 / -40 with -10 / -10 within 2 km (7.3). **D22:** garage vehicles destroyed (9.1). **D23:** uniform holds, #86 narrows capture only, the town_defenders edit (8.1, 12.2). **D24:** the HQ meter at `HqPos`, independent of zone ownership, re-establish anywhere, no zone census exception (9.1, 8.1). **Critique 5:** N1 (one HQ model: default camp HQ, `HasHq` / `HqZone`, `_hqDestroyed`, no silent Camp re-HQ, recruit anchor on `gmHqPos`); N2 (bases in push-out and on held-zone decay, 6.3, 6.4); N3 (D2 runs in SPENT, single exit at 10 %, 5.6); N4a (phase-1 `HqZone()` dispatch skip, 9.1, 14); N4b (D23). MINOR: live fights against all present defenders, `A_lethal` / `A_bodies` (6.5, 5.5); `occCapture` field and `abstractOcc` for BASE records (7.2); per-zone hold groups, income item moot (7.2); `townStruck` / `struckUntil` and the bought-when rule (7.3); 1.6 h best case (5.2); airframe removal gated beyond the edge (3.2); APC before Tank (6.5); U5 key-off refund (3.3); cache emptied at the fall (9.1); `_hqEpoch` marker re-arm (9.1); `legacySave` flag and migrated saves at 0 tokens (11). Section 17 emptied. |
| 2026-10-13 (orchestrator, phase 5) | **Armour escorts a carrier.** A column is a carrier (truck or APC) plus at most one escort (armed car, APC or tank); a lone tank or a two-seat armed car is never chosen (`seatsShort`), only bought as an escort; a launch-time foot downgrade refunds the whole column (3.2, 6.5). |
| 2026-10-13 (orchestrator, phase 4 accepted deviations) | `baseRetake` follows `occupierForces` unless authored; resistance alone decays `occCapture` at `captureDecayDefended`; "in a wave" means a wave inbound or landed within 1800 s; the final-stand surge is excluded from spare only in the director's sources; a base route is the road A*, else the straight cross-country line, which counts as reaching the target; a START TOWN HQ keeps the #79 B2 exemption, a re-established HQ never gets it (3.1, 6.3, 7.1, 7.2, 9.1). |
| 2026-10-14 (orchestrator, phase 6) | **Aircraft "unseen" in 3D.** For removal, an aircraft is unseen out of the view cone, terrain-masked at altitude, or beyond the perception band in 3D even inside the cone; `departTimeout` (900 s) stays as the backstop (3.2). The air spike's verdict is GO (`AIR-SPIKE.md`). |
| 2026-10-14 (reconciliation) | The code wins: 3.1 (immediate re-source, the road-route join, the wait, the edge hold, legacy records on the chain), 3.2 (air recipe, columns, the dismount), 5.1 (K without the probe), 5.5 (routing rows), 6.1 (informant tips), 6.3 (final push, wave HQ candidate, born cap), 6.5 (M1, the base minimum cap), 6.6 (instruments), 7.1 (allocation order, tokens, no hub), 7.2 (`occCapture` rates, every holding force, `RevertBase`), 9.1 (the HQ meter and default HQ), 9.2 (had something to lose, the lose test), 11, 12, 16 and 17 brought to the committed code. |
| 2026-10-14 (phase 6, `bcca80e`: the landmass finding) | **Malden's Southeast Compound is an island.** About 800 m of deep sea part it from Le Port; the road-route join's cross-country leg from it crossed the sea, and its W and S gates are closed wire gates; the squad stall inside its west gate (open since phase 3) is resolved with it. The rule: a ground force never comes from, and a Traffic vehicle never drives to, a base on another landmass (`RoadRoute::Landmass`, 50 m geography cells joined over land, a road over water a bridge); the join lands only on the source's landmass; such a base is an AIR-ONLY source (`PackageSource::airOnly`), and the birth chain re-plans as an insertion at once when no other base shares the target's landmass. This replaces the phase 4 deviation's "the straight cross-country line counts as reaching the target" for bases off the target's landmass. The offscreen fixture's SEBase moved to the Le Port quay (3.1). |
| 2026-10-14 (LOW review fixes after `08c6ae2`) | **Known bisect gap (L15):** `8f9482a` does not build on its own (it includes `CounterAttackOrders.hpp` and calls `LaunchFunded`, which arrive in `7a143ec`); history is not rewritten, so a bisect skips `8f9482a` (`git bisect skip`). The live top-up cap supersedes the phase 6 deviation "a live alarm answer adopts its whole squad". |
| 2026-10-14 (phase 6 accepted deviations, `bcca80e`) | No attack-helicopter escort for an insertion (one airframe per insertion); a live alarm answer adopts its whole squad (no `garrisonMax` cap on the live path); strikes are live only in the bubble, abstract elsewhere; wave strikes buy no air opener; an HQ strike with no report aims at `HqPos`; a base-opener strike cuts only the 2 km ring; the air re-plan's helicopter price bypasses the director ledger; alarm answers take no token and run while DORMANT (otherwise a key-on campaign has no answer before the second town). Open with them: the descriptor lint does not check the `forces*` keys; a slow native answer could contest a later Showcase capture chapter (17). |
| 2026-10-14 (orchestrator, after `402ece6`) | **The settlement probe returns for K** (5.1). A seeded CITY counts in K and G only when the Auto seed's probe (dry land, 3 house-sized buildings within 300 m, 16) accepts it; authored towns always count. The verdict is probed once and saved on the zone row, so K is stable across sessions. Deviation: a block saved before the probe keeps its saved K (re-sizing a running pool would break the ledger, 5.5); only a save without the block probes on load. Measured: Abel K 14 (unchanged), Sinai K 37 to 26, poolStart 88 to 66. |
| 2026-10-14 (owner decisions) | **Director lines are dev-only (D12 kept).** The per-action INFO lines of the director, `CounterAttack` and `OccupierAir` log at INFO only under `--dev` or with `forcesLog = 1` (default 0), and at DEBUG otherwise; WARN and ERROR are unchanged (6.6, 16). Journal integration is GearUnclear/CWR#98, form open (17). **Alarm answers are born at their base** and travel at real speed even when the target is beside the player (3.1 step 1 as built; the open item is closed). **The Southeast Compound stays an air-only base** for now; boats wait for the vehicle system (17, parked). |
| 2026-10-17 (owner decision, round 6, video 7 op 4) | **The reserve floor.** A director-bought ground operation may not take the pool under `forcesPoolFloor` x poolStart by its own debit; it waits whole (no token, rest or re-arm spent, one line per target and plan) and is never downgraded below what its plan required, though the plan's own cheapest qualifying package may go when it fits; its plan is chosen on max(pool, poolStart) with the floor kept. Exempt: HQ assaults, wave operations, strikes (their own band rules, unchanged), alarm answers to bases. Implementer's calls: a helicopter insertion is floored like any retake (it has no band rule of its own; only strikes have one), an alarm answer to an occupier TOWN is floored (a garrison top-up), and the waits are not saved (6.3) |

## 19. Residual risks the owner accepts

1. **A watched road means you watch them come.** Forces are born farther
   back, out of view, and arrive in sight. When every road is watched, the
   operation is quietly called off.
2. **Passing enemies wear you down.** A patrol or column through a town you
   left empty retakes it, and each pass weakens it until you come back (D5,
   D18). A patrol that kills the squad you left at a base takes the base
   (D19).
3. **Your HQ can be destroyed, wherever it is.** A deliberate assault takes
   it. The weapon cache goes with it, and so do the vehicles in its garage
   and your recruiting point. You may set up a new one anywhere, for free.
   An HQ inside an enemy town does not make that town any easier to take.
4. **The campaign can be lost.** You lose if your HQ falls while you hold no
   base and no town, or if you lose your last base or town while you have
   no HQ. It ends in a defeat debriefing. Your saves stay, and a save taken
   in that state reloads into the same defeat.
5. **Bases change hands more often.** The occupier goes for your bases when
   it is strong, and waves can take them back, so a win needs every enemy
   base held at the same moment.
6. **Enemy aircraft can bomb your towns, and everyone loses.** People die
   and the town panics. Both your support and the occupier's standing there
   collapse, the towns around it suffer too, and a bombed town of yours
   falls to the next enemy who walks in. In return the occupier gets fewer
   informants there and rebuilds more slowly.
7. **Answers on foot are slow, motorized and air answers fast and
   expensive.** About 40 minutes against a few minutes over the same road.
8. **Helicopters depend on the AI landing reliably.** If landings prove
   unreliable, the ground packages still do every job.
9. **A town retaken in front of you looks empty** until you leave or its new
   garrison can arrive unseen. Garrisons can still pop in at 800 m with a
   raised view distance (older behaviour).
10. **Old campaigns get the core only, and you see the occupier only in the
    log for now.** They keep the old QRF and heat cooling until a new
    campaign. Player messaging is a later step.
