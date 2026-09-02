# Guerrilla Mode — status (Phase 1.5: native core + event-driven scripts)

Maps every Phase-1 "Three Zones" requirement from
[`../mod-plans/13-guerrilla-mode.md`](../mod-plans/13-guerrilla-mode.md) to
where it lives NOW: **native** (engine `engine/Poseidon/Game/Guerrilla/` +
evaluator, landed in commit `3ef5cd5`) or **script** (the event-driven policy
layer, one copy, under `core/`). The Phase-1 all-SQS implementation of
rows marked *native* is deleted; gameplay behavior is preserved.

*(Everything below is "written, pending a build+run" — code-complete and
reconciled against the engine source, not play-tested.)*

| # | Requirement | Now lives in | Kind | Notes |
|---|-------------|--------------|------|-------|
| 1 | Zone table (Camp / Village / Outpost) | `description.ext` `CfgGuerrillaZones` → engine `ZoneRegistry` | **native (data-driven)** | Per-island data file (issue #3 item 3); positions in getPos order; `seedCities=1` auto-adds every named town as a CITY zone |
| 2 | Marker recolor/label on flip + fog reveal | `ZoneRegistry::UpdateMarkers` | **native** | Engine repaints only EXISTING markers; `init.sqs` creates one marker per zone (once — created markers serialize with the world) |
| 3 | Territory capture (military clear→flip; CITY on support) | `ZoneRegistry::EvaluateTick` | **native** | Player-proximity (cacheRadius) gate, heat spike, income tap all engine-side; fires the `captured`/`supportThreshold` events |
| 3b | Capture REACTION (hold garrison + notify) | `scripts/capture.sqs` | **script** | holdCount-sized role-mixed squad via `gmFactionSquad` (plan 15); tier-less descriptors keep holdClass × holdCount |
| 4 | Outpost garrison: officer-first groups, SENTRY/GUARD | `GarrisonCache::SpawnGarrison` | **native** | Role-diverse squads (plan 15): `ZoneRegistry::FactionSquad` template over tiers[] + tiersMG/AT/Medic/Sniper[]; officer key; descriptor classnames package-resolved at load; reads script `gmWarLevel` |
| 5 | Distance-cached spawn/despawn + survivor write-back | `GarrisonCache` | **native** | +50 m despawn hysteresis; `garrisonSpawned`/`garrisonDespawned` events; `gmGarrisonForceDespawn` escape hatch |
| 6 | QRF: spawn + convoy on RED, road-snap, stall fallback | `scripts/qrf.sqs` | **script (policy, by design)** | Reacts to `alertChanged`; classes/vehicle/officer via `gmFaction*`, side via `gmOccupierSide`; behavior preserved from old spawning.sqs |
| 7 | Graduated alert FSM (GREEN/YELLOW/RED) | `AlertMachine` | **native** | knowsAbout bands, disengage window, last-known pos (`gmZoneLastKnown`), edge heat spikes; garrison POSTURE reaction (behaviour edges + YELLOW investigate moves) stays in `qrf.sqs` |
| 8 | War Level + spawn tiers | `scripts/escalation.sqs` (ladder) + faction `tiers[]` (data) | **script + data** | Same ladder formulas; tier selection is native `gmFactionTierClass` |
| 9 | Per-region Heat with decay | native raise / `scripts/escalation.sqs` decay | **split (engine-enforced)** | No direct heat setter: `gmZoneSet heatRaise/heatDecay` only, clamped; decay still gated on GREEN |
| 10 | ₽/HR economy tick | `scripts/economy.sqs` | **script** | Formulas unchanged; reads `gmZoneCount`/`gmZone`, owner vs `gmResistanceSide` |
| 11 | Recruit / specialist / train action menu | `scripts/recruit.sqs` + `recruit_action.sqs` | **script** | Flow unchanged; Camp anchor = first CAMP-type zone; classes from faction keys |
| 12 | Loot-on-kill stash + gear unlock | `scripts/loot.sqs` | **script** | Logic unchanged; bodies tagged via `gmGarrisonGroups` per spawned zone; loadout classnames from faction `loot<Role>*` keys |
| 13 | Named companion XP→rank/skill, permadeath | `scripts/companions.sqs` | **script** | Model unchanged; `companionClass` from faction key; promotions now apply LIVE `setRank` (evaluator has it) |
| 14 | Persistence: save/load round-trip | engine (`ZoneRegistry`/`AlertMachine`/`GarrisonCache` `Serialize` + GGameState globals) | **native** | Event handlers serialize too; `campaignLoaded` event replaces the GM_SAVED sentinel; `scripts/campaign.sqs` keeps the Save action + reconciles companion handles / `GM_PLAYER_GROUPS` |
| 15 | Undercover (disguise) | engine `UndercoverSystem` (per-observer perception, 2026-07-16) + `AlertMachine` (compromise Heat/event) + `scripts/undercover.sqs` (establish + advisory hint) | **native** | Each occupier group independently reads the player as civilian / suspect (`TSideUnknown`, investigates) / exposed, from weapon state (in hands / slung / none), facing, distance, visibility and its own serialized compromise memory (permanent per group by default); vehicles get their own policy (civilian car anonymous; stolen military vehicle friendly at range, suspect at high accuracy, exposed close; a witnessed getaway marks the vehicle for those witnesses only); `gmUndercover` + `setCaptive` stay script-owned and are never dropped on a break; re-establishment is emergent (stow weapon, avoid or eliminate witnesses); query via `gmUndercoverStatus`/`gmUndercoverWitnesses` |
| 16 | Swappable factions | `CfgGuerrillaFactions` + `gmOccupierSide`/`gmResistanceSide` | **native (data-driven)** | Zone owner accepts `OCCUPIER`/`RESISTANCE`/`NEUTRAL` tokens; new-game UI publishes `gmSelOccupier`/`gmSelResistance` |
| 17 | Island-agnostic scripts | `scripts/*` | **done** | Zero classnames / side-string literals in `scripts/` (grep-verified); all such facts live in `description.ext` |
| 18 | Town side flags: map icon + physical flagpole | map: `init.sqs` ("Flag" markers) + `ZoneRegistry::UpdateMarkers` (side color); world: engine `TownFlags` | **native** | One `FlagCarrier` per CITY zone, off-road (RoadNet probe) on high ground toward the outskirts; texture from faction `flag` key, else side default (usa/ussr/fia), else generic white; repaints on flip; serializes (`GuerrillaFlags`); FlagCarrier-less packages degrade to markers-only |
| 19 | Persistent arms stashes (keep-when-empty holders) | engine `ResourceSupply::_keepWhenEmpty` + `StashRegistry` (`gmStash*`) | **native** | Flag serialized on the holder (presence-tolerant, old saves unchanged); registry rows serialize as `GuerrillaStashes`, save-gated on non-empty so it works outside Guerrilla missions; dead holders pruned on a 5 s tick |
| 20 | Character outfit family: warrior vs civilian select, recruits auto-match (issue #25) | engine (`UI/Guerrilla` cycler idc 153 + `Game/Guerrilla/OutfitSelect` player substitution + `civTier[]`/`gmFactionCivTier`) + `scripts/` (`GM_OUTFIT_CIV` fold, `*Civ` key reads) | **native + script** | Locked at new-game (`gmSelOutfit`; WARRIOR ≡ publish-nothing); descriptor keys `playerClass{Warrior,Civ}`, `{recruitFighter,recruitSpecialist,companionClass,holdClass}Civ`, `civTier[]` — see ARCHITECTURE.md A.5; civilian hold squads are a `holdClassCiv` monoculture (no `tiersCiv[]` yet); saves round-trip free via the GameState bank |
| 21 | Player-body BODY browser: pick any side's Man class as the player's body | engine (`UI/Guerrilla` cycler idc 155 + `GuerrillaListPlayerBodies` roster + `OutfitSelect::ResolvePlayerBodyClass`) | **native** | Class-driven follow-up to issue #25's vocabulary question, player-only (squads stay on the outfit family); publishes `gmSelPlayerClass` (exact classname, `(match outfit)` default publishes nothing); pick beats the outfit token, probe failure keeps the authored class; config side reads as that side at distance (accepted emergent, undercover untouched) — see ARCHITECTURE.md A.5 |
| 22 | Field journal on the map screen: Notes / Plan pages (field manual, live Situation block, diary, objectives, next steps) | engine `Game/Guerrilla/Journal` (+ `JournalCommands.cpp`: `gmJournalLog` / `gmJournalObjective` / `gmJournalStatus` / `gmJournalCount` / `gmJournalEntry` / `gmJournalObjectiveState` / `gmJournalStatusText` / `gmIslandName`) + `UI/Guerrilla/GuerrillaJournalPages` (page renderer hooked into `DisplayMap::ReloadBriefingContent`) + `scripts/` diary/objective/status writes | **native + script** | No briefing.html needed: when `CfgGuerrillaZones` is active the map's Notes page (`Main`/`__BRIEFING`) is built natively on every map-key open (`DisplayMap::ResetHUD` seam; header, Situation block read live from ZoneRegistry/AlertMachine/UndercoverSystem/StashRegistry + the script economy globals, a 10-topic field manual on `GM_MAN_*` pages, the latest diary lines + a full `GM_LOG` page) and the Plan page gets the standing goal, the engine-derived next steps and the scripted objectives; the open map repaints on every journal change (revision compare) and on the Notes/Plan tab press; diary/objectives/status serialize as `GuerrillaJournal` (save-gated on non-empty, presence-tolerant on load, old saves unaffected); managers write the diary (capture arc, QRF, cover blown, promotions/deaths, unlocks, War Level edges, recruits, save/restore) and four starter objectives; island name comes from `gmIslandName` (CfgWorlds description), never a literal |
| 23 | Ambient road traffic: civilian cars town-to-town, occupier patrols between posts, rare supply convoys; commandeer a civ car; road murders feed the civ kill ledger | engine `Game/Guerrilla/Traffic` (+ `TrafficCommands.cpp`: `gmTraffic*`, `gmRoadNearest` / `gmRoadPath` / `gmRoadsNear` + `nearestRoads`) + `init.sqs` handler lines + `civVehicles[]` in the CIV descriptor | **native** | Player-distance band [300, 1500] m (+300 m despawn hysteresis), caps 3/1/1, one-roll rarest-first spawn per 5 s pass, convoy chance war-scaled (cap 0.3); routes are doMove-style `IssueCommand` Moves to the drivers (CARELESS civ / SAFE occupier keep road pathing), arrival re-dispatch while watched, stall teardown only out of sight; commandeer = in-lane-ahead or armed-aim inside 25 m -> Stop -> 2.5 s -> driver bails + flees, hull released (deleted when far unless boarded); civ drivers carry the `driverKilled` killed-EH expression (`GM_fnCivKilledEH`); serializes as `GuerrillaTraffic`; `trafficEnabled=0` switches it off; patrol/convoy traffic feeds the AlertMachine (2026-09-01, closed the accepted gap): a violent end (destroyed/crewDead) queues a `TrafficAmbush` the alert tick drains into a per-zone knowsAbout floor (patrol = YELLOW band held steady, convoy = RED band, decaying over `trafficAmbushWindow`=120 s, `trafficAmbushHeat`=4 per wreck, lastKnown = the wreck; attribution: nearest occupier zone within `trafficAmbushRadius`=1500 m, else the origin zone while the occupier still holds it, else dropped), and a live traffic crew between zones is attributed to the nearest occupier zone within `trafficAmbushRadius` instead of being discarded; spawn-chance modulation (2026-08-25): a pure `ModulationFactors` pre-stage scales the civ/patrol chances before the band subtraction - wall-clock day trapezoid (`trafficCivNightScale`=0.1 outside `trafficDayStart`=0.25..`trafficDayEnd`=0.875, 2 h ramps), civ-route-origin alert (RED zeroes civ, YELLOW 0.4×, patrols ×(1+`trafficAlertPatrolBoost`=0.5) on YELLOW/RED), curfew (war ≥ `trafficCurfewWarLevel`=3 + `NightEffect` > 0.5 + occupier-owned origin: civ 0, patrol ×`trafficCurfewPatrolBoost`=2.0), rain fade (`trafficRainCivFade`=0.6); neutral defaults keep noon/GREEN/war-1/dry behaviour identical and `gmTrafficForceSpawn` still bypasses the roll; headlights needed **no code**: `TransportCore`'s auto-light gate already lights AI crews at night under the CARELESS/SAFE modes Traffic issues, and douses them on combat escalation |
| 24 | Headquarters: elected start town or in-mission election, weapon cache, 100 m vehicle garage with lockable (beep-beep, invulnerable) persistent vehicles, paid moves (issues #16 M1+M4, #28) | engine `Game/Guerrilla/GuerrillaBase` (+ `GuerrillaBaseCommands.cpp`: `gmHq*` / `gmGarage*`), `UI/Guerrilla/GuerrillaNewGame` START TOWN cycler (idc 152, `gmSelStartTown`), `ZoneRegistry::CollectTownNames`; policy in `scripts/market.sqs` (+ `market_action.sqs`) | **native + script** | One HQ per campaign in any zone: the best enterable building of the zone (Paths LOD, `hqMinPos`>=4 AI positions, most positions then nearest the centre) holds the cache indoors with the garage ring 20-50 m beside it, a zone without one (the Camp, a hamlet) falls back to an off-road dry spot on the outer rings of the zone area (cache + garage together); the cache is a keep-when-empty `WeaponHolder` registered as a stash (retrieval = the holder's own TAKE actions), moving the HQ moves it with its contents; any Transport inside `garageRadius` (100 m) can be locked (`gmGarageLock`): lock + `allowDammage false` re-asserted every 2 s tick (neither is serialized), a hull that leaves 1.5x the ring is released, the horn muzzle plays two short bursts; the new-game cycler lists exactly the CITY zones the campaign will carry (authored + seeded) and the first tick establishes the HQ there and relocates the player; serializes as `GuerrillaBase` (save-gated on the registry being active, rows by `SerializeRef`, `autoTried` one-shot) |
| 25 | Money sinks: arms + vehicle dealers drawn over the towns, delivery to the HQ (issue #27) | engine `Game/Guerrilla/Market` (+ `MarketCommands.cpp`: `gmMarket*` / `gmDealer*`) fed by `class CfgGuerrillaMarket` in `description.ext`; purchases in `scripts/market.sqs` | **native + script** | At the first tick each kind goes to `round(cities * dealerShare)` (>=1) CITY zones, drawn independently (one town may host both) from a seed drawn once and serialized; each dealer is a CIV NPC (`dealerClass` > the CIV descriptor's `civClass1` > `Civilian`, package-probed) on a deterministic off-road spot (weapon dealers on the cardinal bearings, vehicle dealers on the diagonals, a LOT spot >=15 m away for delivered hulls), `DAMove`/`DATarget`/`DAAutoTarget` disabled, respawning after `dealerRespawnSeconds`; stock rows (weapon + magazines, weapon only, magazine-only bundles; vehicles) are package-probed at load and dropped non-fatally, display names from the package config; `market.sqs` mounts the BUY menu beside a live dealer (<=8 rows + a here/HQ delivery toggle), debits `gmResources` (the second "-" writer next to recruit.sqs), drops a `WeaponHolder` at the player's feet or fills the HQ cache, parks a hull on the dealer's lot or drops it into the HQ garage locked; serializes as `GuerrillaMarket` (rows by zone name + NPC refs, the seed); `gmDealerStock` / `gmDealerNearest` for scripts |

## File map (current)

| File | Role |
|------|------|
| `description.ext` | THE per-island data file: `CfgGuerrillaZones` (tuning + zone seed + `seedCities=1`) and `CfgGuerrillaFactions` (Demo: stock EAST/GUER classes) |
| `core/init.sqs` | thin bootstrap: script-state seed, zone markers, 8 one-line native handler registrations, exec managers. Lives ONCE at `guerrilla-mode/core`, installed to `<GameDir>\gmcore`; each mission's own `init.sqs` is the two-line `[] exec "\gmcore\init.sqs"` |
| `core/scripts/lib.sqs` | 8 helpers (`GM_fnRandPosNear/SpawnGroup/SpawnSquad/SideFromString/CountOwnedBy/ZoneOfType/FactionNum/BumpGear`) + `GM_LIB_READY` |
| `core/scripts/capture.sqs` | `captured` consumer: hold garrison + "liberated" hint; diary lines for ready/lost/liberated + the `firstZone`/`firstTown` starter objectives |
| *(engine)* `Game/Guerrilla/Journal.*`, `JournalCommands.cpp`, `UI/Guerrilla/GuerrillaJournalPages.*` | the field journal behind the map's Notes/Plan pages (row 22): diary / objectives / status tables (`gmJournal*`), the page renderer + field-manual text; `scripts/` only WRITE to it (campaign: begin/save/restore; qrf: QRF launch; undercover: cover blown; companions: promotion/death + `Companions` status line; loot: unlock + `Unlocked gear` status line + `firstUnlock` objective; escalation: War Level edges; recruit: recruit/train + `firstRecruit` objective) |
| *(engine)* `Game/Guerrilla/Traffic.*`, `TrafficCommands.cpp` | ambient road traffic (row 23): civ cars / occupier patrols / convoys, commandeer, road queries; `init.sqs` registers 4 enqueue handlers + the `driverKilled` ledger expression |
| *(engine)* `Game/Guerrilla/GuerrillaBase.*`, `GuerrillaBaseCommands.cpp`, `Game/Guerrilla/Market.*`, `MarketCommands.cpp`, `UI/Guerrilla/GuerrillaNewGame` (START TOWN cycler) | headquarters / cache / garage (row 24) and the dealer market (row 25): election + siting + the keep-when-empty cache + garage locks + the start-town election; dealer draw + NPCs + stock (`class CfgGuerrillaMarket` in `description.ext`) |
| `core/scripts/market.sqs` / `market_action.sqs` | the money-loop action menus over those facts: Establish/Move HQ (debits `hqMoveCost` on a move), Stash at the cache, Lock/Unlock in the garage, the BUY menu beside a dealer (here / HQ delivery) - the second `gmResources` debit writer; map markers, `hqEstablish` objective, diary lines |
| `core/scripts/qrf.sqs` | `alertChanged` consumer: garrison posture, YELLOW investigate, RED QRF convoy |
| `core/scripts/undercover.sqs` | cover establish (kept for the whole campaign), fired-EH → `gmBreakUndercover`, advisory `undercoverBroken` hint (never drops captive) |
| `core/scripts/campaign.sqs` | Save addAction + `campaignLoaded` consumer (companion/group reconciliation) |
| `core/scripts/economy.sqs` / `escalation.sqs` / `loot.sqs` / `recruit.sqs` / `recruit_action.sqs` / `companions.sqs` | unchanged responsibilities on the native surface |
| *(deleted)* `zones.sqs`, `spawning.sqs`, `alert.sqs`, `persistence.sqs` | polling loops replaced by the native systems |

## Behavior deviations from the Phase-1 scripts (deliberate, documented)

1. **YELLOW investigate target** is the native last-known position
   (`gmZoneLastKnown`) instead of the old `getPos aP` approximation — the
   honest value the old code had a `VERIFY` for. QRF's final SAD waypoint
   likewise prefers last-known over the zone center.
2. **Live `setRank` on companion promotion** (was: rank visible only after the
   next respawn — the evaluator lacked the command).
3. **Post-load companion/loot handles are trust-but-verified** instead of
   unconditionally nulled: a GGameState link that deserialized valid and alive
   is kept (no duplicate spawn); dead/null links are rebuilt from the rows.
4. **Hold-garrison group handles are not tracked** (old `GM_HOLD_GROUPS` was
   write-only).
5. **`seedCities=1`** adds every named town as a CITY zone: town income and
   the War-Level denominator now scale with the island (balance knob, not a
   bug; the three-zone core loop is unchanged).
6. `revealed`/`garrisonDespawned` events are registered and drained but
   trigger no extra UX. `supportThreshold` now has its own beat ("ready to
   rise") — it is decoupled from the flip since the consolidation-capture
   rework (2026-07-11).

## Consolidation capture (2026-07-11 rework)

Instant capture is gone. Military zones carry a serialized 0..100 **capture
meter** (`gmZone` element 9): it climbs per tick per attacker (crew-capped)
only while NO live occupier unit stands inside `zoneArea` — positional,
side-wide presence, so QRF/patrols/mission troops all contest (the old
`liveOccupiers` bookkeeping decides nothing). Contested = frozen; defenders
alone re-secure at `captureDecayDefended`; abandoned progress fades at
`captureDecayAbandoned`. CITY support accrues only in an occupier-free town,
occupier-only presence bleeds it back toward `supportDecayFloor`, and the
flip needs fighters standing in an occupier-free town at/past the threshold —
no more spontaneous flips, no more "liberating" a patrolled town. The
undercover player counts for neither side. `captureRate=100` restores the
legacy instant flip per island/zone.

Pre-consolidation saves load unchanged (the `capture` field is
presence-tolerant) and gain the new mechanics; they do NOT gain the new
narration — running SQS scripts resume their serialized text, so the three
new capture events fire into the empty handler slots such saves carry.
Accepted degradation, engine-side safe.

## Known edges (source audit 2026-07-06, updated 2026-07-11 — revisit with play feel)

- **Occupier-owned CITY zones are capturable via support since 2026-07-11**
  (underground organizing; the old permanently-dead edge is fixed). Third-
  side-owned cities remain out of reach by design.
- **Meters freeze outside `cacheRadius` (800 m)** — no gain, no decay, no
  contest while the player is away (world-bubble policy, deliberate).
- **Hidden-straggler watch item**: a lone surviving defender inside the zone
  silently freezes the meter (marker flips to CONTESTED/white as the tell);
  `contestOutnumberRatio` covers the inverse case (defenders held hostage by
  one hidden attacker). Verify in play that returning patrols + DEFENDED
  decay clean up half-captured litter.
- **Nothing captures while the player is dead/absent** (`playerValid` gate);
  garrisons freeze rather than despawn. Correct for single-player Phase 1.
- **Alert FSM edges**: RED→YELLOW re-arms the escalation window, so a
  partially-visible player oscillates RED↔YELLOW; there is no calming grace
  period below `alertYellowKnows`; occupier groups farther than `zoneArea`
  from every zone center contribute no `knowsAbout` unless they are a live
  Traffic crew (see the closed ambient-traffic gap below), so a running
  firefight *between* zones with a non-traffic garrison group still raises
  no alert; an undercover compromise heats the zone
  nearest the *witness* regardless of distance.
- **Overflow recruit groups are dropped from `GM_PLAYER_GROUPS` on load**
  (`campaign.sqs` reseeds to `[group aP]`) — deliberate Phase-1 policy; cells
  larger than one group lose their extra-group bookkeeping across a save.
- **Undercover compromise is per-observer-group and permanent by default**
  (2026-07-16 rework; `undercoverForgetSeconds=0` is the decay knob). Groups
  that never saw the break keep reading a civilian, so re-establishment is
  emergent: stow the weapon, break contact, or eliminate the witnesses. The
  player `fired`-EH stays attached for the whole campaign, and
  `gmBreakUndercover` with zero current witnesses latches silently (no
  event, no Heat).

- **Ambient traffic alert gap: CLOSED (2026-09-01)**: occupier patrol
  vehicles and convoys now feed the `AlertMachine` two ways. A violent end
  (destroyed/crewDead) queues a `TrafficAmbush` stimulus the alert tick
  drains into a per-zone `knowsAbout` floor with a `trafficAmbushWindow`
  (120 s) decay - a patrol wipe holds the YELLOW band steady (the disengage
  window bleeds into RED only under a live contact), a convoy wipe the RED
  band, and
  the zone's lastKnown becomes the wreck position, so qrf.sqs reacts with
  zero script changes; each drained ambush also raises `trafficAmbushHeat`
  (4) on the zone, so repeat ambushes inside a held window still cost.
  Attribution is the nearest occupier zone within `trafficAmbushRadius`
  (1500 m, deliberately decoupled from the `trafficRadius` player band),
  falling back to the route's origin zone if the occupier still holds it,
  else the stimulus is dropped. And a live `Traffic::IsTrafficGroup` crew
  under fire between zones (outside every `zoneArea`) is attributed to the
  nearest occupier zone within `trafficAmbushRadius` instead of being
  discarded. The stimulus queue and per-zone floors serialize. Still open:
  the civ kill ledger consumer
  drops kills farther than `GM_CIV_EFFECT_R` (300 m) from the attributed
  town (`civilians.sqs`), so a deep-countryside road murder is written to
  `gmCivKilled` but stays unpunished for now. And group-level `AIGroup::Move`
  / arcade ACMOVE waypoints never moved a freshly seated vehicle crew in
  probes (SendCommand delivery; the crew sat with `unitReady` false forever)
  - Traffic issues its route legs as direct `IssueCommand` Moves to the
  drivers instead; worth a look if another system ever needs waypoint
  convoys.

- **Convoy discipline under fire (2026-09-01)**: convoy Move commands now
  land their speed mode and `FormColumn` on the command subgroup they
  actually create (the constructor defaults were wedge at `SpeedNormal`, so
  the engine's native convoy-follow never engaged); escorts seat up to two
  cargo riflemen and are watched for quiet loss; an escort dead within
  `trafficBailCombatWindow` (60 s) of the group's last disclosure makes the
  truck crew bail (`bailed` event, player-caused remains); and a fighting
  patrol/convoy holds the whole trip ladder (stall accrual, blocked
  recovery, arrival/stall endings, re-legs) under a bounded combat gate
  (`trafficCombatStaleAfter` 120 s / `trafficCombatHoldMax` 300 s) instead
  of being lingered or torn down mid-fight. Known edge: `combatHold`
  accrues in pass intervals, not wall seconds, matching `stallTime`.

- **Civ danger response (2026-09-01)**: civilian traffic reacts to nearby
  gunfire, blasts and fresh player-caused wrecks. Two fast-gated one-call
  hooks in the frozen core (`EntityAI::FireWeaponEffects`,
  `Landscape::ExplosionDammage`, `GTrafficDangerArmed` in the
  `GUndercoverActive` mold) feed a coalescing 8-slot ring; a pure
  per-entry reaction roll picks cower (`TSPanicked`, resumes when quiet),
  U-turn home at full speed, rush past, or bail-and-run from the danger
  point. One reaction per `trafficDangerCooldown` (45 s); the commandeer
  always wins; patrols/convoys are excluded (native combat AI owns them).
  Keys: `trafficDangerRadius` (200 m at severity 1, 0 = off),
  `trafficDangerCloseRadius` (60), `trafficDangerTtl` (20). Probe-gated
  residual: whether a CMCareless driver visibly accelerates on the rush
  slot; disabling it is a one-constant change (`DangerFarRushBand`).

## Tests

The engine systems carry Catch2 unit coverage (ZoneRegistry / AlertMachine /
GarrisonCache pure cores, config parsing, side resolution, seedCities). The
Trident integration suite is **fully migrated to the `gm*` surface** (verified
2026-07-06 — the old `GM_ZONES`/`zones.sqs` spine survives only in comments):

| Test | Data needed |
|---|---|
| `scripting/guerrilla_capture_flip.test` | Demo world + GUER roster ‡ (headless) |
| `scripting/guerrilla_spawn_domove.test` | Demo world + GUER roster ‡ (headless, `gate-zero`) |
| `scripting/guerrilla_save_reload.seq` (01_save → 02_reload) | Demo world + GUER roster ‡ (headless, `save-load`) |
| `scripting/guerrilla_native_capture.test` | `full_cwa` |
| `scripting/guerrilla_native_spawn.test` | `full_cwa` |
| `scripting/guerrilla_native_undercover.test` | `full_cwa` |
| `scripting/guerrilla_undercover_rules.test` | `full_cwa` |
| `scripting/guerrilla_native_save_reload.seq` | `full_cwa`, `save-load` (+ journal sentinels since 2026-08-22) |
| `scripting/guerrilla_journal_pages.test` | `full_cwa` (opens the real map display: Notes/Plan/GM_LOG/GM_MAN_* pages, link routing, live repaint) |
| `scripting/guerrilla_sinai_swap.test` | `full_cwa` + `lobo` (+ fixture gen + installed templates) |
| `ui/guerrilla_new_game_e2e.test` | `full_cwa` + `lobo` (+ installed templates) |
| `scripting/qrf_reference_mission.test` | `full_cwa` (boots `guerrilla-mode/mission/Qrf.Abel` directly: native garrison -> forced reveal -> YELLOW -> RED -> qrf.sqs convoy -> perception removed -> GREEN -> stand-down) |
| `scripting/guerrilla_traffic_ambient.test` | `full_cwa` (force-spawned civ car on the road, CIV driver, drives, offshore teleport drains the table + `despawned`) |
| `scripting/guerrilla_traffic_commandeer.test` | `full_cwa` (armed player in the lane: Stop -> driver bails -> hull released, `commandeered`, player boards) |
| `scripting/guerrilla_traffic_patrol.test` | `full_cwa` (Camp flipped to the occupier, patrol hull from the Outpost on the road, occupier crew, drives) |
| `ui/main_menu/reference_mission_{showcase,undercover,qrf,market}.test` | `full_cwa` (+ installed templates; the main-menu direct-launch buttons idc 123/124/125/126) |
| `scripting/market_reference_mission.test` | `full_cwa` (boots `guerrilla-mode/mission/Market.Abel` directly: dealers drawn per kind = round(cities x dealerShare), NPCs alive -> buy here (WeaponHolder at the feet, debit) -> Establish HQ in the nearest town (CITY, building, stash registered, objective DONE) -> Stash the rifle / retrieve / holder survives empty -> delivery to HQ: weapon into the cache cargo, vehicle into the garage locked -> gmGarageLock unlock/relock -> Move HQ to the Camp: outdoor fallback, hqMoveCost debited, cache travels with cargo, the far hull released) |
| `ui/guerrilla_start_town_e2e.test` | `full_cwa` (+ installed templates; START TOWN cycler idc 152 on Abel -> "Village" -> launch -> `gmSelStartTown`, HQ standing in Village, player relocated beside it, `hqEstablish` DONE) |

‡ These three bind to the `demo` world (folder suffix `.Demo`) and their player
+ resistance units are GUER classes (`SoldierGB`, …). The local remaster Demo
package `Arma Cold War Assault Demo [Remaster]` ships `demo\demo.wrp` (=
`abel.wrp`) but **no `SoldierG*` roster** (only `SoldierGFakeE`; issue #13), so a
`tri --data-dir "…Demo [Remaster]"` run boot-fails on `SoldierGB` (strict-mode
fatal, "harness connection closed", ~60 ms; verified 2026-07-08). They run only
once a Demo-package faction descriptor remaps the resistance onto Demo's
EAST/WEST classes, or the missions' Gate-Zero class substitution lands. On this
machine, meanwhile, Classic has no `demo` world — so these are not runnable on
either local package yet, and the runnable Guerrilla set stays the `full_cwa`
tests.

Unit-level serialization round-trips exist for all five native systems
(`test_zone_registry.cpp`, `test_alert_machine.cpp`, `test_garrison_cache.cpp`,
`test_town_flags.cpp` (the latter also pins the flag-texture resolution chain
and the off-road/high-ground spot picker), `test_stash_registry.cpp`, and
`test_journal.cpp` (diary cap, objective/status upsert, save/load round-trip,
plus the Notes/Plan/diary/manual page renderer against a parser-only HTML
container and an authored-Main/Plan append case));
`test_mission_script_core.cpp` enforces the ONE-core shape (issue #54 step B1,
which retired the N-way byte-identical copy walk): the core is
`guerrilla-mode/core`, its `scripts/` set equals the manifest ARCHITECTURE.md
A.6 documents, no mission template or `guerrilla_*` test mission carries a
`scripts/` directory, every full-core mission's `init.sqs` is the two-line
`[] exec "\gmcore\init.sqs"` bootstrap, every `\gmcore\scripts\<x>.sqs`
reference resolves to a real core file, and no relative `"scripts/` reference
survives. The `Qrf.*` / `Market.*` / `Undercover.*` reference missions keep
their own bootstrap and exec the one core policy script they demonstrate, so
each sandbox runs the campaign's real script by construction, never a copy.
`guerrilla_native_save_reload.seq` additionally stamps a diary line, an
objective flip and a status line in phase 01 and diffs them after the
cross-process reload in phase 02 (the `GuerrillaJournal` save block), and
since 2026-08-22 also elects the HQ in the Village, locks a fresh Jeep in the
garage, puts a rifle in the cache and remembers the dealer draw, diffing all
four after the reload (the `GuerrillaBase` / `GuerrillaMarket` save blocks:
the hull ref resolves, lock + invulnerability are re-asserted on the load
pass, the cache holder keeps its cargo, the seeded towns come back).

Headquarters + market (2026-08-22) unit coverage: `test_guerrilla_base.cpp`
pins the HQ building picker (most AI positions, nearest breaks ties), the
outdoor spot picker (road/water/occupied rejects, caller ring order), the
2D range test, the `hqMinPos`/`garageRadius`/`garageInvulnerable` parse with
floors, the unestablished query set and the `GuerrillaBase` save/load
round-trip (established state by value, tuning rebuilt from the reparsed
mission config, null garage rows dropped, the unestablished round-trip);
`test_market.cpp` pins `DealerQuota`, the seeded/salted `ShuffleOrder`, the
dealer plan (quota per kind, overlap at share 1.0, authored town overrides,
no cities), kind names, the unconfigured market, the stock parse through a
fake package probe (weapon+mags / weapon-only / magazine-only rows, unknown
weapon or magazine rows dropped or sold bare, price floor, the dealer-body
fallback chain, a CfgMagazines bank), `NearestDealer`, and the
`GuerrillaMarket` round-trip (rows by zone name with the ghost town dropped,
seed + tuning). `test_zone_registry.cpp` gains `CollectTownNames` (authored
CITY zones first, seeded Names towns only under `seedCities`, the 300 m and
name dedup, the type filter) and `test_journal.cpp` the Headquarters /
Dealers Situation lines.

Undercover (2026-07-16 rework) integration coverage:
`guerrilla_native_undercover` rewritten to the new lifecycle (captive and
`gmUndercover` persist across a break; asserts status/heat instead), the new
`guerrilla_undercover_rules` exercises the per-observer rule matrix on the
`full_cwa` lane, and `showcase_smoke`'s chapter-6 asserts flipped to the new
semantics. Unit-level half: `test_undercover.cpp` covers both evaluator rule
matrices (person and vehicle, 474 assertions incl. boundary and war-scaling
cases), and `test_alert_machine.cpp` was repaired to the new contract
(compromise drain, silent no-witness latch, `undercoverHeatWitness`).

Ambient traffic (2026-08-22) coverage: `test_traffic.cpp` pins the tuning
parse, the one-roll rarest-first spawn decision (caps, chances, disabled),
the war-scaled convoy chance + cap, the far-despawn hysteresis, the route
picker (civ CITY->CITY band + fallback, occupier pairs, pinned origin,
nothing in range), the farthest-in-band spawn point, the commandeer trigger
matrix (ahead-in-lane / beside / behind / armed aim cone / out of radius),
stall expiry, event/kind name mapping and the `GuerrillaTraffic` save/load
round-trip with null-hull row pruning and an empty-archive load; the three
`guerrilla_traffic_*` Trident tests above cover the world half, and
`guerrilla_native_save_reload.seq` carries a forced civ car's row across the
cross-process reload.

Traffic spawn-chance modulation (2026-08-25) coverage: `test_traffic.cpp`
adds the seven new `traffic*` tuning keys to the parse defaults/override
sections and pins the pure `ModulationFactors` core (neutral-default
identity with the unmodulated bands, the day trapezoid's night/ramp/plateau
segments, alert scaling, each leg of the curfew predicate, rain fade,
multiplicative composition + the [0,1] civ clamp under silly tuning) plus
`DecideSpawn` applying the scales before the band subtraction (civScale 0
removes the civ band, a scaled patrol band shifts the civ band start
exactly, a 100× patrol boost caps at certainty). The world fill (wall
clock, null-guarded `NightEffect`/rain reads, civ-route-origin alert) rides
the existing `guerrilla_traffic_*` Trident tests, which force-spawn past
the roll and so cannot flake on modulation.

## Still needs a human run

The Phase-1 acceptance play-through stands: boot, clear the Outpost, watch the
flip + hold garrison, income tick, recruit, loot toward the AK unlock, take a
companion through a promotion, trip a QRF, then Save/load and confirm the
campaign resumes (companions rebuilt, markers intact, garrisons re-cached).
