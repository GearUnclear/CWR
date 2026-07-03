# Guerrilla Mode — status (Phase 1.5: native core + event-driven scripts)

Maps every Phase-1 "Three Zones" requirement from
[`../mod-plans/13-guerrilla-mode.md`](../mod-plans/13-guerrilla-mode.md) to
where it lives NOW: **native** (engine `engine/Poseidon/Game/Guerrilla/` +
evaluator, landed in commit `3ef5cd5`) or **script** (the event-driven policy
layer under `mission/Guerrilla.Demo/`). The Phase-1 all-SQS implementation of
rows marked *native* is deleted; gameplay behavior is preserved.

*(Everything below is "written, pending a build+run" — code-complete and
reconciled against the engine source, not play-tested.)*

| # | Requirement | Now lives in | Kind | Notes |
|---|-------------|--------------|------|-------|
| 1 | Zone table (Camp / Village / Outpost) | `description.ext` `CfgGuerrillaZones` → engine `ZoneRegistry` | **native (data-driven)** | Per-island data file (issue #3 item 3); positions in getPos order; `seedCities=1` auto-adds every named town as a CITY zone |
| 2 | Marker recolor/label on flip + fog reveal | `ZoneRegistry::UpdateMarkers` | **native** | Engine repaints only EXISTING markers; `init.sqs` creates one marker per zone (once — created markers serialize with the world) |
| 3 | Territory capture (military clear→flip; CITY on support) | `ZoneRegistry::EvaluateTick` | **native** | Player-proximity (cacheRadius) gate, heat spike, income tap all engine-side; fires the `captured`/`supportThreshold` events |
| 3b | Capture REACTION (hold garrison + notify) | `scripts/capture.sqs` | **script** | holdClass × holdCount from the resistance faction descriptor |
| 4 | Outpost garrison: officer-first groups, SENTRY/GUARD | `GarrisonCache::SpawnGarrison` | **native** | Tier class from `CfgGuerrillaFactions` tiers[]/thresholds; officer key; reads script `gmWarLevel` |
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
| 15 | Undercover (disguise) | `AlertMachine` (break detection) + `scripts/undercover.sqs` (establish/react) | **split** | `gmUndercover` script-owned; vehicle-mount break polled natively, fired-weapon half via `gmBreakUndercover`; break now fully wired (was the Phase-1 gap) |
| 16 | Swappable factions | `CfgGuerrillaFactions` + `gmOccupierSide`/`gmResistanceSide` | **native (data-driven)** | Zone owner accepts `OCCUPIER`/`RESISTANCE`/`NEUTRAL` tokens; new-game UI publishes `gmSelOccupier`/`gmSelResistance` |
| 17 | Island-agnostic scripts | `scripts/*` | **done** | Zero classnames / side-string literals in `scripts/` (grep-verified); all such facts live in `description.ext` |

## File map (current)

| File | Role |
|------|------|
| `description.ext` | THE per-island data file: `CfgGuerrillaZones` (tuning + zone seed + `seedCities=1`) and `CfgGuerrillaFactions` (Demo: stock EAST/GUER classes) |
| `init.sqs` | thin bootstrap: script-state seed, zone markers, 8 one-line native handler registrations, exec managers |
| `scripts/lib.sqs` | 7 helpers (`GM_fnRandPosNear/SpawnGroup/SideFromString/CountOwnedBy/ZoneOfType/FactionNum/BumpGear`) + `GM_LIB_READY` |
| `scripts/capture.sqs` | `captured` consumer: hold garrison + "liberated" hint |
| `scripts/qrf.sqs` | `alertChanged` consumer: garrison posture, YELLOW investigate, RED QRF convoy |
| `scripts/undercover.sqs` | cover establish, fired-EH → `gmBreakUndercover`, `undercoverBroken` consumer |
| `scripts/campaign.sqs` | Save addAction + `campaignLoaded` consumer (companion/group reconciliation) |
| `scripts/economy.sqs` / `escalation.sqs` / `loot.sqs` / `recruit.sqs` / `recruit_action.sqs` / `companions.sqs` | unchanged responsibilities on the native surface |
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
6. `supportThreshold`/`revealed`/`garrisonDespawned` events are registered and
   drained but trigger no extra UX (a CITY flip already fires `captured` →
   one "liberated" hint, matching Phase 1).

## Tests

The Phase-1 integration tests (`tests/integration/scripting/guerrilla_*`) were
written against the old script spine (they poke `GM_ZONES` etc.); the engine
rewrite ships its own unit coverage for ZoneRegistry/AlertMachine/GarrisonCache
(pure `EvaluateTick`/`Decide` cores). The mission-level tests need a rewrite
against the `gm*` command surface — tracked separately; **not** part of this
mission migration (tests/ is owned by other agents).

## Still needs a human run

The Phase-1 acceptance play-through stands: boot, clear the Outpost, watch the
flip + hold garrison, income tick, recruit, loot toward the AK unlock, take a
companion through a promotion, trip a QRF, then Save/load and confirm the
campaign resumes (companions rebuilt, markers intact, garrisons re-cached).
