# Guerrilla Mode — Phase-1 MVP status

Maps every Phase-1 "Three Zones" MVP requirement from
[`../mod-plans/13-guerrilla-mode.md`](../mod-plans/13-guerrilla-mode.md) to the
file that implements it and its state. All paths are under
`guerrilla-mode/mission/Guerrilla.Demo/` unless noted.

Legend: **Done** = implemented with confirmed commands, cross-file wired.
**Partial** = works but a piece is stubbed/deferred. **Todo** = not started.
*(Everything is "written, pending a build+run" — see README "verified vs needs
an in-game run"; "Done" here means code-complete + reconciled, not play-tested.)*

| # | Plan-13 Phase-1 requirement | File(s) | Status | Notes |
|---|------------------------------|---------|--------|-------|
| 1 | `ZONES[]` of 3 (Camp / Village / Outpost) | `init.sqs` (`GM_ZONES`, `GM_Z_*`) | **Done** | Seed positions **fixed to getPos order** this pass (see Reconciliations). |
| 2 | Capture triggers + **marker recolor on flip** | `zones.sqs` | **Done** | Per-zone `createTrigger` EmptyDetector + authoritative loop decision; markers recolor by (revealed,owner). Covered by `guerrilla_capture_flip` test. |
| 3 | Territory capture (military clear→flip; CITY on support) | `zones.sqs` | **Done** | Flip gate reconciled with spawning (GM_Z_GAR zeroed on spawn) so clearing the garrison actually flips. |
| 4 | Outpost garrison, sentry/guard waypoints, ~8 men / two groups / one officer | `spawning.sqs` | **Done** | Groups of ≤6 + officer (≤12/group cap); SENTRY (officer) / GUARD; `setBehaviour`/`setCombatMode`. |
| 5 | Distance-cached spawn/despawn (DAC), survivor write-back | `spawning.sqs` | **Done** | Spawn zeros `GM_Z_GAR`; despawn writes survivors back (spec §3.6). |
| 6 | One QRF: spawns + convoys on RED, road-snap + spawn-on-arrival fallback | `spawning.sqs` | **Done** | `nearestObject` road-snap (objNull fallback), `move`+MOVE/SAD waypoints, teleport-on-stall insertion. |
| 7 | Graduated alert state machine (GREEN/YELLOW/RED) | `alert.sqs` | **Done** | Per-zone max-`knowsAbout` banding; YELLOW disengage window; RED signals QRF via `GM_ALERT_STATE`. |
| 8 | War Level, **two spawn tiers** | `escalation.sqs` + `lib.sqs` (`GM_fnPickTier`) | **Done** | Owned-% ladder; WL≥3 crosses tier-1→tier-2 that `spawning.sqs` reads. |
| 9 | Per-region Heat with decay | `escalation.sqs` (decay) + `zones.sqs`/`alert.sqs` (raise) | **Done** | Directional-split ownership (raise vs decay); decay gated on GREEN. |
| 10 | ₽ Resources + HR Manpower on a ~10-min tick | `economy.sqs` | **Done** | GUER military income + CITY support→₽/HR; HR hard-capped. |
| 11 | Recruit-via-action-menu (recruit / specialist / train) | `recruit.sqs` + `recruit_action.sqs` | **Done** | `addAction` gated by Camp proximity; dispatcher enqueues, loop is single writer; 12-cap overflow to new group. |
| 12 | Loot-on-kill stash + **one gear unlock** (25 AK mags → AK) | `loot.sqs` (+ `lib.sqs` `GM_fnBumpGear`) | **Done** | `killed` EH tally through the sole `GM_GEAR_*` writer; unlock at `GM_GEAR_THRESHOLD`=25; `GM_fnEquipFromUnlocked` public. |
| 13 | **One** named companion, XP→rank/skill, permadeath | `companions.sqs` | **Done** | "Petra"; XP fold (kill via `rating` delta + survival); live `setSkill`; rank baked at (re)spawn (`setRank` absent); permadeath keeps the row. |
| 14 | Persistence: `saveGame`/`loadGame` round-trip | `persistence.sqs` + engine `GGameState` serialization | **Done** | Globals serialize for free; persistence rebuilds only transient handles. Covered by `guerrilla_save_reload` seq test. |
| 15 | Undercover (disguise) hook | `escalation.sqs` (establish) | **Partial** | `gmUndercover`/`gmUndercoverDetect` established + `setCaptive true`; the **break** condition is a Phase-2 hook (not yet wired in `alert.sqs`). Plan 13 lists full undercover under Phase 2. |
| 16 | Land plan **09** `enableAI` | `engine/…/GameStateExtUi.cpp` (+ `GameStateExt.cpp` reg) | **Done, needs compile/run** | Scripts degrade without it (`disableAI "MOVE"` freeze path). |
| 17 | Land plan **10** `knownTargets` | `engine/…/GameStateExtGrp.cpp` (+ `GameStateExt.cpp` reg) | **Done, needs compile/run** | Scripts degrade without it (single-object `knowsAbout` polling in `alert.sqs`). |

## Tests (this pass)

| Requirement it guards | Test | Status |
|-----------------------|------|--------|
| Gate-Zero spawn + move (plan 13 GZ #2) | `tests/integration/scripting/guerrilla_spawn_domove.test.{sqf,toml}` | **Written** |
| Capture flip + marker recolor (#2,#3) | `tests/integration/scripting/guerrilla_capture_flip.test.{sqf,toml}` | **Written** |
| Save/reload round-trip (plan 13 GZ #3, #14) | `tests/integration/scripting/guerrilla_save_reload.seq{.toml,/01_save,/02_reload}` | **Written** |
| `enableAI`/`knownTargets` registered + mask logic (#16,#17) | `tests/unit/…/test_game_state_ext.cpp` `[game][gameStateExt][ai]` | **Written** |
| Gate-Zero framerate eyeball (GZ #1) | manual (README checklist) | **Todo (human run)** |

## Reconciliations applied this pass (cross-file integration fixes)

1. **Coordinate order (systemic, high-impact).** `GM_ZONES` seed positions in
   `init.sqs` were copied verbatim from the `mission.sqm` `position[]`, which is
   `[easting, elevation, northing]`. Every script consumes `GM_Z_POS` in **getPos
   order** `[easting, northing, elevation]` (engine `ObjGetPos`: `array[1]=Z`
   northing, `array[2]=Y` up; `GetRelPos` maps `array[1]→Z`, `array[2]→Y`). The
   y/z were therefore swapped, silently breaking *all* distance checks, marker
   placement, and spawn positions. **Fixed:** swapped indices 1↔2 of all three
   seed positions to getPos order.
2. **Capture could never fire (spec §3.6 half-implemented).** `spawning.sqs`
   read `GM_Z_GAR` to size a spawn but never zeroed it, so a zone read
   `GM_Z_GAR>0` forever (the tight 3-zone cluster never leaves the 800 m radius to
   trigger the despawn write-back), and `zones.sqs`'s `GM_zCached<1` capture gate
   could never be satisfied. **Fixed:** zero `GM_Z_GAR` on spawn — spec §3.6's
   "crossing the radius *converts* between the two" — with despawn restoring it.

## Not-drift, but worth noting
- `GM_COMP_CLASS` is defined identically in both `companions.sqs` and
  `persistence.sqs` (a self-contained fallback each; same value → harmless).
- Trigger objects are held by handle in `GM_ZONE_TRIG` rather than by the
  spec's illustrative `gmZoneTrig_<i>` name; nothing references them by name, so
  there is no cross-file mismatch.
