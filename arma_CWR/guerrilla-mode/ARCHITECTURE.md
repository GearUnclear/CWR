# Guerrilla Mode — Architecture Spine (binding spec)

This is the **canonical structure every build agent conforms to.** It fixes the
mission layout, the global-variable schema, the bootstrap contract, the
per-manager API (read/write/owns) so there are no collisions, and the SQS/SQF
dialect decision. It is grounded **only** in commands confirmed by reading the
real engine source and shipped missions in this repo (citations inline). Where a
fact could not be confirmed it is marked **`VERIFY`** — never invent commands.

Master design: `mod-plans/13-guerrilla-mode.md`. C++ dependencies: plan
`09-sqf-enable-ai.md` (`enableAI`) and `10-sqf-known-targets.md` (`knownTargets`)
— both MVP-cheap, both currently **unimplemented**, scripts degrade gracefully
until they land.

> **Scope correction (2026-07-01):** this spec binds the **Phase-1
> `Guerrilla.Demo` mission only** — it is not the game's architecture. Two of
> its foundations are transitional, imposed by "prove the loop on an unmodified
> engine," and are scheduled for retirement in the Phase-1.5 de-hardcoding pass
> (plan 13, "Core requirement: swappable factions & islands"):
>
> 1. **Hardcoded factions/island.** The `"GUER"`/`"EAST"` side strings, stock
>    classnames (`SoldierGB`/`SoldierEB`, the AK unlock), zone seed coordinates,
>    and the Demo world are demo fixtures. The real game selects island +
>    civilian/resistance/occupier factions at new-game; all such facts move to
>    faction descriptors and per-island data files.
> 2. **Route-arounds for missing evaluator features.** The parallel name-keyed
>    arrays (no `setVariable`), single-line code-strings (no `compile`),
>    `GM_tmp*` scratch globals, and `GM_LIB_READY` sentinel (no `isNil`) are
>    workarounds, not idioms to propagate. The project is a total engine
>    overhaul; those commands get added engine-side, and post-Phase-1 scripts
>    are written against the upgraded evaluator.

---

## 0. Dialect decision: **SQS**, with SQF code-strings as synchronous helpers

**Managers and bootstrap are SQS (`.sqs`).** Evidence:

- `init.sqs` is auto-loaded by `RunInitScript` (`engine/Poseidon/UI/DisplayUI.cpp:155-162`); `init.sqf`, if present, is loaded *after* but runs **unscheduled** — it has no `~` scheduler, so it cannot host the timed tick-loops the meta-game needs.
- SQS gives the three primitives the loops require, all seen in shipped missions:
  - `~N` waits (`demo_end_tail_success.Demo/scenario.sqs:1`),
  - `@cond` wait-until (used here for the lib handshake),
  - `goto "label"` + `#label` — **`goto` is a registered command** (`GameStateExt.cpp:1056`, `ScriptGoto`), so `#loop … goto "loop"` is valid for persistent managers.
- `[args] exec "file.sqs"` spawns a script (`exec` = `GameOperator` at `GameStateExt.cpp:1320`; shipped usage `mission.sqm:41,87`).

**Helpers are SQF code-strings.** Critical ground-truth correction to the recon:
in this evaluator a `{ … }` literal **is a `GameString`**, and the body operands
of `call` / `while` / `do` / `then` / `else` / `forEach` are **`GameString`**, not
a distinct code type (`engine/Evaluator/express.cpp:1132-1149,1187-1194`). So:

- `[args] call GM_fnX` runs the string with `_this` bound and **returns the last expression's value** (synchronous, non-interruptible — no `~` allowed inside).
- `while {cond} do {body}`, `if (b) then {a} else {b}`, `"…_x…" forEach arr` all work because the braces are just string literals.
- **`compile` does not exist** (recon-confirmed) and is not needed — we never turn a `preprocessFile` string into code; helpers are authored inline as `{…}`.

Consequence for `lib.sqs`: each helper is a **single-line** `GM_fnX = { … }`
assignment (SQS is line-oriented; keep code-strings on one physical line), and
the file ends by setting `GM_LIB_READY = true`. Because `exec` is **asynchronous**,
`init.sqs` must `@GM_LIB_READY` before launching managers (load-bearing handshake).

Other confirmed-absent items the spec routes around: `doMove` (declared, **not
registered** — use group `move`/`addWaypoint`), `nearestObjects` (loop
`nearestObject`), `rank`/`setRank` and `setVariable`/`getVariable` (use parallel
globals keyed by name), `isNil` (use the `GM_LIB_READY` sentinel bool), `enableAI`
+ `knownTargets` (plans 09/10, unimplemented).

---

## 1. Mission directory layout

World = **`Demo`** (extracted from the folder name after the final dot —
`MissionPathLoader.hpp:86`; every shipped test mission uses `.Demo`, e.g.
`mp_assign_report.Demo`). The task template `Guerrilla.<island>.Demo` resolves
to `<island>` = the Demo world, i.e. the folder below. **`VERIFY`**: if a richer
named Demo island is acquired later, rename the folder's final segment to that
world and update nothing else (world comes from the folder, not `mission.sqm`).

```
guerrilla-mode/
├─ ARCHITECTURE.md                         ← this spec
└─ mission/
   └─ Guerrilla.Demo/                      ← world "Demo"
      ├─ mission.sqm        minimal valid SQM: one GUER player soldier at the Camp
      ├─ description.ext     mission metadata (onLoadMission, respawn=0, no JIP)
      ├─ init.sqs            BOOTSTRAP: defines ALL globals, execs lib, execs managers
      └─ scripts/
         ├─ lib.sqs          shared synchronous helpers (GM_fn*) + GM_LIB_READY handshake
         ├─ zones.sqs        territory model: markers, capture detection, ownership flip
         ├─ spawning.sqs     distance-cached garrison/QRF spawn + despawn (DAC pattern)
         ├─ alert.sqs        graduated GREEN/YELLOW/RED knowsAbout state machine
         ├─ escalation.sqs   War Level (slow) + per-region Heat decay (fast)
         ├─ economy.sqs      Resources (₽) + Manpower (HR) income tick (~10 min)
         ├─ loot.sqs         loot-on-kill stash + per-classname gear-unlock tally
         ├─ recruit.sqs      addAction command layer: recruit / train / gear / fast-travel
         ├─ companions.sqs   named-companion XP→rank/skill, permadeath
         └─ persistence.sqs  on-load rebuild of transient live objects from GM_* globals
```

**Single-responsibility rule:** exactly one script may *write* any given global
(the "OWNS" column in §4). Other scripts read it, or mutate only through a
helper. This is what prevents collisions across the parallel build effort.

---

## 2. Naming convention (binding)

| Prefix    | Meaning                                              | Persisted? | Examples |
|-----------|------------------------------------------------------|-----------|----------|
| `GM_*`    | constants / tunables / structured data (UPPER words) | yes (data) | `GM_ZONES`, `GM_Z_OWNER`, `GM_GEAR_THRESHOLD` |
| `gm*`     | mutable scalar faction state                         | yes        | `gmResources`, `gmManpower`, `gmWarLevel` |
| `GM_fn*`  | code-string helpers (defined in `lib.sqs`)           | no (redefined each load) | `GM_fnDist2D`, `GM_fnSpawnGroup` |
| `GM_tmp*` | scratch globals used inside helpers                  | no (never read) | `GM_tmpG`, `GM_tmpI` |
| `GM_*OBJ`/`GM_CACHE_*`/`GM_PLAYER_GROUPS` | **transient** live handles | **NO — rebuilt on load** | `GM_COMP_OBJ`, `GM_CACHE_GROUPS` |

**Persistence rule (from design):** persist **IDs/classnames, never object
handles** — handles break across `saveGame`/load. All `*OBJ` / `*GROUPS` /
`GM_CACHE_*` arrays are transient and reconstructed by `persistence.sqs`.

---

## 3. Global-variable schema (exact names + shapes)

### 3.1 Zones — `GM_ZONES` (array of records)

Each element is the design's `ZONE` tuple (`13-guerrilla-mode.md:132`):

```
ZONE = [name, type, owner, garrisonStrength, support, income, heat, markerName, [x,y,z]]
```

Accessed through index constants (defined in `init.sqs`, used everywhere):

| Const          | Idx | Type   | Domain / meaning |
|----------------|-----|--------|------------------|
| `GM_Z_NAME`    | 0   | string | unique zone id |
| `GM_Z_TYPE`    | 1   | string | `"CAMP"`/`"AIRFIELD"`/`"SEAPORT"`/`"OUTPOST"`/`"CITY"` |
| `GM_Z_OWNER`   | 2   | string | `"GUER"`/`"EAST"`/`"NEUTRAL"` (side string, not a side value — avoids handle issues) |
| `GM_Z_GAR`     | 3   | scalar | authoritative occupier body count **while despawned/cached** |
| `GM_Z_SUPPORT` | 4   | scalar | 0..100 town support (CITY flips on this, not on kills) |
| `GM_Z_INCOME`  | 5   | scalar | ₽ per economy tick when GUER-owned |
| `GM_Z_HEAT`    | 6   | scalar | 0..100 **per-region Heat** (fast timescale) — stored on the zone, not a separate array |
| `GM_Z_MARKER`  | 7   | string | marker name this zone owns: `"gmZoneMarker_<i>"` |
| `GM_Z_POS`     | 8   | array  | `[x,y,z]` center |

Phase-1 seed (3 zones): `Camp` (GUER/CAMP), `Village` (NEUTRAL/CITY, support 20),
`Outpost` (EAST/OUTPOST, garrison 8, income 25).

### 3.2 Player faction scalars

| Global        | Type   | Phase-1 start | Owner writer | Meaning |
|---------------|--------|---------------|--------------|---------|
| `gmResources` | scalar | `100`         | economy.sqs (+), recruit.sqs (−) | ₽ treasury |
| `gmManpower`  | scalar | `2`           | economy.sqs (+), recruit.sqs (−) | HR; 1 HR = 1 body; hard faction cap |
| `gmWarLevel`  | scalar | `1`           | escalation.sqs | 1..10 slow escalation; gates spawn table |
| `gmHeatDecay` | scalar | `1`           | (const, read by escalation.sqs) | Heat bled per region per tick |

### 3.3 Per-region Heat

Lives **in the zone record** at `GM_Z_HEAT` (no parallel array). **Raisers:**
`zones.sqs`, `alert.sqs` (append-only += spikes). **Decayer:** `escalation.sqs`
only. This split is the collision-avoidance contract for Heat.

### 3.4 Gear unlocks — parallel arrays + unlocked set

| Global              | Shape                | Owner writer | Meaning |
|---------------------|----------------------|--------------|---------|
| `GM_GEAR_THRESHOLD` | scalar (`25`)        | const        | loot N of an item → unlock |
| `GM_GEAR_ITEMS`     | `[classname,…]`      | loot.sqs (via `GM_fnBumpGear`) | tallied classnames |
| `GM_GEAR_COUNT`     | `[scalar,…]`         | loot.sqs (via `GM_fnBumpGear`) | index-aligned tallies |
| `GM_GEAR_UNLOCKED`  | `[classname,…]`      | loot.sqs (via `GM_fnBumpGear`) | permanently unlocked & infinite set |

`GM_GEAR_ITEMS[i] ↔ GM_GEAR_COUNT[i]` are strictly index-aligned. All three are
mutated **only** through `[class,amount] call GM_fnBumpGear`.

### 3.5 Named companions — parallel arrays keyed by name

Index `i` is one companion across all arrays (no `setVariable` namespaces exist).

| Global            | Shape (per-i)                         | Persisted | Meaning |
|-------------------|---------------------------------------|-----------|---------|
| `GM_COMP_NAMES`   | string                                | yes (key) | identity key (permadeath keeps the row) |
| `GM_COMP_ALIVE`   | bool                                  | yes       | living / permadead |
| `GM_COMP_RANK`    | string (enum name)                    | yes       | passed to `createUnit` rank arg / `SetRank` |
| `GM_COMP_XP`      | scalar                                | yes       | awarded on kills/survival |
| `GM_COMP_SKILL`   | scalar 0..1                           | yes       | applied via `setSkill` |
| `GM_COMP_LOADOUT` | `[ [weaponClass…], [magClass…] ]`     | yes       | re-applied via `removeAllWeapons`+`addWeapon`/`addMagazine` |
| `GM_COMP_OBJ`     | object handle or `objNull`            | **NO**    | **transient** live handle, rebuilt on load |

Phase-1 seed: one companion `"Petra"` (CORPORAL, skill 0.55).

### 3.6 Cache / spawn bookkeeping (all **transient**)

| Global             | Shape                 | Owner writer | Meaning |
|--------------------|-----------------------|--------------|---------|
| `GM_CACHE_GROUPS`  | `[group,…]`           | spawning.sqs | live occupier garrison groups |
| `GM_CACHE_ZONEIDX` | `[scalar,…]`          | spawning.sqs | index-aligned: which zone each group holds |
| `GM_PLAYER_GROUPS` | `[group,…]`           | recruit.sqs  | player-faction live groups (12-cap overflow) |
| `GM_CACHE_RADIUS`  | scalar (`800`)        | const        | spawn-in/despawn distance around player (m) |

**Cache state machine (design-critical):** despawned garrison = the integer
`GM_Z_GAR`; spawned garrison = transient groups in `GM_CACHE_*`. Crossing the
radius converts between the two; on despawn, survivor count is written **back**
to `GM_Z_GAR`.

### 3.7 Alert state (transient, index-aligned to `GM_ZONES`)

| Global           | Shape        | Owner writer | Meaning |
|------------------|--------------|--------------|---------|
| `GM_ALERT_STATE` | `[0/1/2,…]`  | alert.sqs    | 0 GREEN / 1 YELLOW / 2 RED |
| `GM_ALERT_TIMER` | `[scalar,…]` | alert.sqs    | YELLOW disengage countdown (s) |

---

## 4. Manager API contract (read / write / owns — the no-collision table)

Exactly one **OWNS** writer per global. "reads" = read-only. Markers/triggers are
namespaced so two scripts never touch the same object.

| Script            | OWNS (exclusive write)                                   | Reads                                              | Owns markers/triggers |
|-------------------|----------------------------------------------------------|----------------------------------------------------|-----------------------|
| `lib.sqs`         | `GM_fn*`, `GM_LIB_READY`, `GM_tmp*`                       | —                                                  | — |
| `zones.sqs`       | `GM_Z_OWNER`,`GM_Z_SUPPORT`,`GM_Z_GAR`; **raises** `GM_Z_HEAT` | `GM_ZONES` pos/type, `GM_CACHE_*`             | `gmZoneMarker_<i>`, per-zone capture triggers `gmZoneTrig_<i>` |
| `spawning.sqs`    | `GM_CACHE_GROUPS`,`GM_CACHE_ZONEIDX`; writes back `GM_Z_GAR` on despawn | `GM_ZONES`, `GM_CACHE_RADIUS`, `gmWarLevel`, `GM_ALERT_STATE` | QRF groups (transient) |
| `alert.sqs`       | `GM_ALERT_STATE`,`GM_ALERT_TIMER`; **raises** `GM_Z_HEAT` | `GM_CACHE_*`, player, `knowsAbout`                 | — (signals spawning.sqs for QRF) |
| `escalation.sqs`  | `gmWarLevel`; **decays** `GM_Z_HEAT`                      | `GM_ZONES` owners, `gmHeatDecay`                   | — |
| `economy.sqs`     | `gmResources` (+), `gmManpower` (+)                       | `GM_ZONES` owner/income/support                    | — |
| `loot.sqs`        | `GM_GEAR_*` (via `GM_fnBumpGear`)                         | dead bodies' `weapons`/`magazines`, player         | — |
| `recruit.sqs`     | `GM_PLAYER_GROUPS`; `gmResources` (−), `gmManpower` (−)   | `GM_GEAR_UNLOCKED`, `gmWarLevel`, companion arrays | player `addAction` IDs |
| `companions.sqs`  | `GM_COMP_XP`,`GM_COMP_RANK`,`GM_COMP_SKILL`,`GM_COMP_ALIVE`,`GM_COMP_OBJ` | `GM_COMP_NAMES`, kill/survival events | — |
| `persistence.sqs` | (transient only) `GM_COMP_OBJ`, `GM_CACHE_*`, `GM_PLAYER_GROUPS`, markers | every persisted `GM_*`/`gm*`          | rebuilds markers on load |

**Heat ownership note:** `GM_Z_HEAT` is the one global with multiple touchers by
design — but writes are partitioned by *direction*: `zones.sqs`/`alert.sqs` only
`+=` (spikes), `escalation.sqs` only `-=` (decay). No script both raises and
lowers it.

---

## 5. Bootstrap contract (`init.sqs`)

1. **Guard/aliases:** `GM_BOOTED = true`; `aP = player`; `gmPlayer = player`
   (mirrors the shipped `aP = player` convention, `mp_assign_report.Demo/init.sqs`).
2. **Index constants:** define `GM_Z_*` (0..8).
3. **Define every global** in §3 with the Phase-1 seed values.
4. **Helpers, then block:** `GM_LIB_READY = false` → `[] exec "scripts/lib.sqs"`
   → `@GM_LIB_READY`. This serializes the async helper load before any manager
   can `call GM_fn*`.
5. **Launch managers:** `[] exec "scripts/<x>.sqs"` for all nine (order not
   load-bearing; each manager re-guards with `@GM_LIB_READY` at its top).
6. `exit`.

`isServer` (`GameStateExt.cpp:907`) and `player`/`isNull` (`:855`,`:931`) **do
exist** if an MP/host guard is later needed; the SP MVP does not gate on them.

### SQS idioms used (all confirmed)
- one statement per line; `~N` to wait N seconds; `@cond` to wait-until.
- `#label` + `goto "label"` (`ScriptGoto`, `GameStateExt.cpp:1056`) for tick loops.
- `? cond : goto "x"` conditional jump (shipped SQS conditional form).
- `[args] exec "file.sqs"` to spawn; `[args] call GM_fn` for synchronous helpers.

---

## 6. `lib.sqs` helper surface (confirmed-command-only)

All are single-line `{…}` code-strings returning their last expression; `call`
is synchronous so they use `GM_tmp*` scratch globals instead of `private`
(unconfirmed keyword).

| Helper                | Signature → result                  | Confirmed primitives |
|-----------------------|-------------------------------------|----------------------|
| `GM_fnDist2D`         | `[posA,posB] → scalar`              | `select`,`sqrt`,`^`,`-` (express.cpp) — the binary `distance` only works object↔object (`:1233`) |
| `GM_fnRandPosNear`    | `[center,radius] → [x,y,0]`         | `random` (`:1166`),`+`,`-`,`select` |
| `GM_fnSpawnGroup`     | `[side,class,count,pos] → group`    | `createGroup` (`:1180`), `createUnit [pos,grp,init,skill,rank]` (`GameStateExtWorld.cpp:237`), `while/do` |
| `GM_fnPickTier`       | `warLevel → unitClass`              | `if/then/else` — **`VERIFY`** EAST classnames vs Demo CfgVehicles |
| `GM_fnCountOwnedBy`   | `ownerString → scalar`              | `count`,`while/do`,`select` |
| `GM_fnZoneIndex`      | `name → index (−1 if none)`         | `count`,`while/do`,`select` |
| `GM_fnBumpGear`       | `[class,amount] → newCount`         | `find`,`set`,array `+` (`:1120,1130,1135`),`count` |

**`createMarker` correction:** it is size-2 `createMarker [name, pos]`
(`GameStateExtWorldConfig.cpp:724-733`), **not** `[x,y,z,name]` as the recon
stated. Managers must use `[name, [x,y,z]]`.

---

## 7. `mission.sqm` (minimal valid)

Copied from `demo_end_tail_success.Demo/mission.sqm` skeleton: `version=11`;
`class Mission { Intel{}; Groups{ one group } }`; `Intro`/`OutroWin`/`OutroLoose`
stubs. One unit: the player at the Camp `[6519.04,149.66,6473.68]` (the exact
known-good spawn used by shipped Demo missions).

**`VERIFY` (player class):** authored as `side="GUER" vehicle="SoldierGB"`
(stock Resistance rifleman) to keep the GUER-vs-EAST side model the whole design
assumes. If the Demo dataset ships no GUER classes, the Gate-Zero agent
substitutes a present class **without flipping the player to WEST**. `.sqm`
parsing tolerates no `//` comments, so this note lives here, not in the file.

---

## 8. What the build agents inherit (do / don't)

- **DO** read/write globals strictly per the §4 OWNS table; mutate `GM_GEAR_*`
  only via `GM_fnBumpGear`; treat all `*OBJ`/`GM_CACHE_*`/`GM_PLAYER_GROUPS` as
  rebuildable.
- **DO** use only commands cited here or freshly confirmed in
  `GameStateExt*.cpp` / `Evaluator/express.cpp`; tag anything unconfirmed
  `// VERIFY:`.
- **DON'T** use `doMove`, `nearestObjects`, `rank`/`setRank`,
  `setVariable`/`getVariable`, `compile`, `isNil` — all confirmed absent; the
  spec routes around each (group `move`/`addWaypoint`, `nearestObject` loop,
  parallel name-keyed globals, `{…}` code-strings, `GM_LIB_READY` sentinel).
- **DON'T** put `~` waits inside a `call`'d helper (call is synchronous); put
  timed logic in the manager `#loop`s.
- **Plans 09/10** (`enableAI`, `knownTargets`) are MVP but unimplemented — write
  against them with a `// VERIFY` fallback (`disableAI "MOVE"` freeze;
  `knowsAbout` single-object polling) so the mission runs today.
```
