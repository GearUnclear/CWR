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

## Known edges (source audit 2026-07-06 — intended for Phase 1, revisit with play feel)

- **An occupier-owned CITY can never be captured**: support accrues only while
  `owner=="NEUTRAL"` and military capture skips CITY zones
  (`ZoneRegistry::EvaluateTick`). Fine for the shipped seeds (cities start
  NEUTRAL); island authors must not set a CITY's `owner="OCCUPIER"` expecting
  it to be takeable.
- **Nothing captures while the player is dead/absent** (`playerValid` gate);
  garrisons freeze rather than despawn. Correct for single-player Phase 1.
- **Alert FSM edges**: RED→YELLOW re-arms the escalation window, so a
  partially-visible player oscillates RED↔YELLOW; there is no calming grace
  period below `alertYellowKnows`; occupier groups farther than `zoneArea`
  from every zone center contribute no `knowsAbout` (a running firefight
  *between* zones raises no alert); an undercover break heats the *nearest*
  zone regardless of distance.
- **Overflow recruit groups are dropped from `GM_PLAYER_GROUPS` on load**
  (`campaign.sqs` reseeds to `[group aP]`) — deliberate Phase-1 policy; cells
  larger than one group lose their extra-group bookkeeping across a save.
- **Undercover break is permanent for the campaign** (re-establishment is the
  Phase-2 hook); the player `fired`-EH stays attached after the break, which
  is harmless because `gmBreakUndercover` is a no-op while
  `gmUndercover==false`.

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
| `scripting/guerrilla_native_save_reload.seq` | `full_cwa`, `save-load` |
| `scripting/guerrilla_sinai_swap.test` | `full_cwa` + `lobo` (+ fixture gen + installed templates) |
| `ui/guerrilla_new_game_e2e.test` | `full_cwa` + `lobo` (+ installed templates) |

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

Unit-level serialization round-trips exist for all three native systems
(`test_zone_registry.cpp`, `test_alert_machine.cpp`, `test_garrison_cache.cpp`);
`test_mission_script_core.cpp` enforces that every test mission's `init.sqs` +
`scripts/` stay byte-identical to the canonical `Guerrilla.Demo` core.

## Still needs a human run

The Phase-1 acceptance play-through stands: boot, clear the Outpost, watch the
flip + hold garrison, income tick, recruit, loot toward the AK unlock, take a
companion through a promotion, trip a QRF, then Save/load and confirm the
campaign resumes (companions rebuilt, markers intact, garrisons re-cached).
