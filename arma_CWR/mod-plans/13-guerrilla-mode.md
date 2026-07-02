# Guerrilla Mode: a persistent open-world insurgency

*Grow a guerrilla cell into the army that liberates an occupied island. Ambush patrols, loot weapons, recruit and train fighters, capture towns from an occupier that escalates as you do. A whole game mode — ~90% SQF mission content over the existing engine, with a small, surgical set of C++ additions.*

> **Status:** master design + architecture plan (pre-MVP). This is the umbrella document; individual C++ pieces it depends on are the existing AI/scripting plans [01–10](./README.md), and future breakouts may become plans 14+.
>
> **Direction correction (2026-07-01):** this project is a **total overhaul of the engine source**, not a mod on a fixed binary. Two consequences override earlier drafts of this document: (1) C++ engine work is a **first-class tool**, not a last resort — never architect script around a missing engine capability that is cheap to add; (2) **swappable factions and islands are a core requirement** — a new game starts by selecting the island, the civilian faction, the resistance faction, and the occupying faction. See the "Core requirement: swappable factions & islands" section.
>
> **Difficulty:** project (multi-milestone), not a single change. The MVP is **intermediate** (pure SQF, no engine edits required). Full fidelity pulls in **advanced** C++ AI work (plans 02–05/08) and an optional UI/persistence layer.
>
> **Path convention:** all paths are **relative to the repo root** (`D:\Arma_CWA\arma_CWR`). The older plans cite `C:/dev/arma_CWR/…`; that location is stale after the 2026-06-28 move — see `CLAUDE.md`.

---

## Summary

The vision: you start as one fighter with a rifle in an occupied Cold War backwater (the *Resistance*/Nogova fantasy). The occupier holds the towns, roads, and airfield; you hold a hidden camp. Over time you grow a faction — more units, trained up, better-equipped from looted gear — and liberate the island one zone at a time, while the enemy gets meaner in response.

This decomposes into **two interlocking loops** the codebase already supports:

- **Tactical loop** (*Scout → Approach → Strike → Loot → Fade*) maps almost natively onto the engine. The AI hierarchy (`AIGroup`/`AISubgroup`/`AIUnit`), waypoint FSM, graded detection (`knowsAbout`), undercover (`setCaptive`), and a rich runtime command surface already provide squads, alarms, reinforcements, and capture detection.
- **Strategic meta-loop** (*territory → faction growth → enemy escalation*) has **no native engine support** (no geoscape, no faction diplomacy, no built-in "town" economy). It is hand-built in SQF over global variables.

**Central architectural decision:** use each layer where it is the better tool. Script stays the layer for content, tuning, and mission logic (edit a file, relaunch — no compile); C++ is the layer for new/missing evaluator commands, data plumbing (faction and island definitions), UI, persistence, and performance. The engine's own philosophy still guides the *runtime* split — *the engine handles reactive low-level combat; mission script drives proactive operational logic* — but "avoid C++" is **not** a rule of this project: the repo is the engine, and plans 09/10 prove a new command costs ~1–2 hours.

A new game begins with a setup selection — **island, civilian faction, resistance faction, occupying faction** — so every faction- or island-specific fact (classnames, zone positions, camp location) must live in swappable data, never inline in scripts.

The genre's three CWA-era proof points all ran on **this** engine generation, and are the blueprint:

- **Resistance** (stock campaign) — men + gear carried across missions via campaign persistence.
- **MFCTI** (Mike-Force Capture The Island) — a town-capture RTS economy written in pure `.sqs`.
- **DAC** (Dynamic-AI-Creator) — distance-cached dynamic spawning to respect engine scale limits.

We are productizing patterns the engine already proved, not inventing a new capability class.

---

## Why it's interesting

This is the highest-ceiling thing you can build on this engine *without art*: it reuses stock units, maps, and vehicles and turns them into an emergent sandbox. The thematic fit is exact — the retail data already ships `resistance.pbo` (Victor Troska's guerrilla campaign on Nogova) and CTI-style MP missions (`Conquerors`, `SectorControl`). And it is the natural consumer of the AI mod-plans already in this folder: plans 02–05/08 are precisely what turn a generic firefight into a *guerrilla* firefight (kill the officer → rout, suppression-and-maneuver, flee-to-cover, radio-net escalation).

---

## The engine reality (verified facts that shape the design)

These were checked against **this** source tree, not generic OFP/CWA lore. Several correct widespread myths:

| Capability | Verdict | Evidence |
|---|---|---|
| Runtime unit/vehicle/group creation | **Present** | `createVehicle`/`createUnit` (`engine/Poseidon/Game/Commands/GameStateExt.cpp:1333-1334`), `createGroup`/`createCenter` (`:1178-1180`) |
| **Dynamic markers & triggers** | **Present** (myth-busted) | `createMarker`/`deleteMarker` (`GameStateExt.cpp:1183-1184`), `createTrigger` (`:1189`), with real backing stores `markersMap` (`GameStateExtWorld.cpp:440+`) and `sensorsMap`. **Zones do NOT have to be hand-authored in the editor.** |
| Marker manipulation | **Present** | `setMarkerPos`/`setMarkerType`/`setMarkerColor` (`GameStateExt.cpp:1245-1248`) |
| Undercover | **Present** | `setCaptive` (`GameStateExt.cpp:1218`) |
| Graded awareness | **Present** | `knowsAbout` returns a 0–4 scalar (`GameStateExt.cpp:1263`) |
| Rank / skill / XP at runtime | **Present** | `setSkill` (`:1381`), `addRating`→`ObjAddExperience` (`:1231`), `Person.SetRank()`/`GetInfo()._experience` runtime-settable |
| Waypoints / orders at runtime | **Present** | `addWaypoint` (`:1417`), `setBehaviour` (`:1297`), full `setWaypoint*` family in `GameStateExtWorldWaypoint.cpp` |
| Global-variable persistence across `saveGame` | **Present** | `World::Serialize` serializes `GGameState` (`World/WorldImpl.cpp:1738`); `GameValue`/`GameVariable`/`GameDataArray` all implement `Serialize` |
| `enableAI` (inverse of `disableAI`) | **Missing** | `disableAI` exists (`GameStateExt.cpp:1307`), no inverse → see plan [09](./09-sqf-enable-ai.md) |
| `setVariable`/`getVariable` namespaces | **Missing — add them** | overhaul stance: implement engine-side; parallel global arrays keyed by unit name are an interim workaround only |
| Engine economy | **Stub** | `AICenter._resources` exists but `SpendResources()` is a no-op → keep the treasury in script globals |
| File I/O from script | **Missing** | persistence is `saveGame` (globals) + optional C++ `SerializeClass` |
| Hard unit caps | **Real** | `MAX_UNITS_PER_GROUP = 12` (`AI/Path/AITypes.hpp:31`); ~63 groups/side; single-threaded sim (~50–100 live AI practical ceiling, **to be measured** — see Gate-Zero) |

Two consequences worth stating up front:

1. Because dynamic markers/triggers exist, the territory layer can **create zones at runtime** rather than forcing a static editor pass — simpler and more flexible than first assumed.
2. Because `GGameState` already serializes into `saveGame`, **single-player persistence is mostly free** — a bespoke C++ `SerializeClass` is an optional robustness/MP upgrade, not a make-or-break prerequisite.

---

## Architecture: the C++ ↔ SQF split

**Use each layer where it is the better tool.** This repo *is* the engine — a total overhaul, not a mod shipped against a fixed binary — so the earlier rule here ("C++ only where structurally impossible from script") is retired. Script keeps the jobs where its iteration speed wins; C++ takes the jobs where a small engine change deletes a large scripting contortion. Concretely, beyond the tactical-AI plans, the engine side now includes as **core** (not optional) work:

- **Evaluator quality-of-life commands** — `setVariable`/`getVariable`, `isNil`, `nearestObjects`, registering the declared-but-unregistered `doMove`, and whatever else Phase 1 had to route around. Each is in the ~1–2 h class (plans 09/10 are the template); collectively they retire the parallel-array/scratch-global idioms.
- **`worldLocations`-style command** — expose the per-world town/location data the map UI already reads (`CfgWorlds >> <world> >> "Names"`, `UI/Map/UIMap.cpp:999`) so the zone graph can auto-seed on any island.
- **New-game setup flow** — island + civilian/resistance/occupier faction selection. The engine still ships the mission-template wizard (`UI/DisplayUIMultiplayerWizard.cpp`, enumerating `Templates\*.<world>` / `SPTemplates\*.<world>`) as a starting point; an in-mission `addAction` setup menu is the acceptable MVP stopgap.

### Lives entirely in SQF / mission content

| System | Script primitives (all confirmed present) |
|---|---|
| Two economies (₽ Resources + HR Manpower) | global variables, arrays, timed loops (`~`, `@`, `time`) |
| Zone/territory ownership model | `createMarker`/`setMarker*`; per-zone global arrays |
| Capture detection | `createTrigger` + area/activation/statements; presence aggregation via `side` + `count units` |
| Garrison & QRF spawning | `createGroup`/`createUnit`/`createVehicle`, `addWaypoint`/`setWaypoint*`, `doMove` convoy chains |
| Recruitment, loadouts, gear unlocks | `removeAllWeapons`/`addWeapon`/`addMagazine`, threshold counters |
| War Level / Heat escalation | scalar globals gating spawn tables |
| Undercover / disguise | `setCaptive` + `primaryWeapon player != ""` checks + civ-class model swap |
| XP / training progression | `setSkill` / `SetAbility`, `Person.SetRank()`, `_experience` store |
| In-mission checkpoints | `saveGame` |

### Requires C++ (the short list)

| Need | Why script can't | Hook point |
|---|---|---|
| `enableAI` command | `disableAI` has no inverse | plan [09](./09-sqf-enable-ai.md); register in `GameStateExt.cpp` binary table (`:1215`, `INIT_MODULE` at `:1468`) |
| `knownTargets <group>` | `knowsAbout` only polls single objects | plan [10](./10-sqf-known-targets.md); reads `AIGroup` TargetList |
| Tactical-AI *feel* | combat scoring / danger FSM / pathfinding internals | plans [04](./04-ai-target-priority.md) (`AIGroupImpl.cpp:834`), [05](./05-ai-suppression-response.md) (`AIUnitImpl.cpp:2190` + `Shots.cpp`), [03](./03-ai-flee-to-cover.md), [08](./08-ai-shared-enemy-intel.md), [02](./02-ai-dynamic-morale.md) |
| Strategic command UI *(optional, late)* | evaluator has no `Display` access | `GameModuleId::Guerrilla` + `GuerrillaDisplay` on `UIMap` — **not** a new app target |
| Robust / MP-ready persistence *(optional)* | no script file I/O | custom `SerializeClass` writing a parallel `.sav` via `GetCampaignSaveDirectory()` |

> **Important correction:** plans 02–05/08 are **not** "optional polish." They deliver the design's #1 pillar ("asymmetry you *feel*"), and they are real AI-internals C++ surgery — budget them as a funded milestone, not garnish. Prototype [04](./04-ai-target-priority.md) first (highest feel-per-line payoff).

---

## Design — Loop 1: the tactical "heist"

Self-contained and satisfying even if the meta-game didn't exist. **Scout → Approach → Strike → Loot → Fade.**

| Beat | Player action | Engine mechanic |
|---|---|---|
| **Scout** | Watch a patrol/garrison, count guns | `knowsAbout` (0–4), distance checks; enemy target memory fades over time so patience pays |
| **Approach** | Stealth vs loud; move undercover | `setCaptive true` while in civ class = non-target |
| **Strike** | Ambush; kill the officer first | target-priority + suppression (plans 04/05); officer death craters group courage → rout (plan 02) |
| **Loot** | Strip weapons/mags off the dead | `weapons`/`magazines` query → `addWeapon`/`addMagazine` into a stash tally |
| **Fade** | Break contact before the QRF | scripted reinforcement timer on detection; survivors flee to cover (plan 03), radio for help (plan 08) |

**Graduated alert (do not ship binary stealth).** A scripted three-state machine polling `knowsAbout` per garrison/patrol:

- **GREEN (calm):** `knowsAbout < 0.5` → sentry/guard waypoints.
- **YELLOW (suspicious):** `0.5 ≤ knowsAbout < 1.5` → investigate last-known pos via `doMove`; a **detection countdown** starts. *This is the disengage window.*
- **RED (alarm):** `knowsAbout ≥ 1.5` or countdown expires → `setBehaviour "COMBAT"`, radio the QRF, raise regional Heat.

This makes detection *recoverable* (the "the plan that goes wrong is the better story" feel) using only polling + a timer.

**Undercover (CWA-native, since there's no loadout inspection).** Disguise = occupying a **civilian unit class** with `setCaptive true`. Cover **breaks** when a primary weapon is selected, you fire, you enter a reported military vehicle, or you fail a roadblock proximity check. Each break does `setCaptive false` and spikes local Heat; detection chance scales with War Level.

---

## Design — Loop 2: the strategic meta-loop

Everything tactical must **buy strategic power**, and strategic power must **change the next fight**.

### Zones as a graph

The island is a set of capturable zones, now creatable **dynamically** (`createMarker`/`createTrigger`):

```
ZONE = [name, type, owner, garrisonStrength, support, income, heat, markerName, [x,y,z]]
```

| Type | Points | Captured by | Yields |
|---|---|---|---|
| **Airfield** | 8 | Force (soft-gated behind War Level 3) | Big income + heavy-vehicle unlock |
| **Seaport / Depot** | 4 | Force | Income + vehicle supply |
| **Outpost / Factory** | 2 | Force (clear & hold) | Income + manpower tick |
| **City / Village** | 1 | **Support, not force** | Manpower (HR) + recruits + intel |

Capture condition for military zones: *all occupier units in the area dead AND a friendly unit present* → flip `owner`, recolor the marker, spawn a friendly holding garrison, open the income/HR taps, reveal adjacent zones. Cities flip on a **support threshold**, not kills.

### Two economies

- **₽ Resources:** ticks every ~10 min from owned outposts/airfields/ports + supportive towns. Spent on training, gear unlocks, vehicles, bribes.
- **HR Manpower:** `1 HR = 1 recruitable body`, generated by supportive towns. This is the hard cap on faction size — and it dovetails with the engine's 12/group / ~100-AI limits, so HR scarcity *is* the performance governor. Design synergy, not a bug.

Both are plain script globals (the engine economy is a stub).

### Faction growth, recruitment & training

The heart of the fantasy, and better-supported than territory:

- **Recruit:** spend HR (+₽ for specialists) → `createUnit` into the player's command (respect the 12-cap; overflow into new groups via `createGroup`).
- **Named companions (Mount & Blade pattern):** a small roster of distinct fighters with persisted skill/loadout. Death is **permanent** — the emotional core.
- **Progression via real engine fields:** track XP in a global keyed by name (no `setVariable` namespaces → parallel arrays); award on kills / survival; at thresholds `SetRank` up and `SetAbility`/`setSkill` up → the soldier visibly shoots better and holds nerve longer.
- **Training (spend ₽):** raise `setSkill` directly as an alternative to combat XP; cap trainable skill at high War Level.
- **No skill *trees*** — engine skill is one float. "Roles" are gated **gear unlocks + class** (a sniper is a unit that gets the scoped-rifle unlock), not a stat tree.

### Gear progression (the Arsenal substitute)

Looted weapons/mags increment a per-classname tally; cross a threshold (default **25**, tunable) → that item is **permanently unlocked & infinite** for the faction, and recruited AI may spawn with it (`removeAllWeapons` + `addWeapon`/`addMagazine` at spawn, role-appropriate + unlocked only). UI is an `addAction` menu, not a dialog arsenal (a minimal `description.ext` dialog is a later upgrade).

### Enemy escalation: War Level + Heat (two timescales)

The single biggest anti-empty-sandbox device.

- **War Level (1–10, slow):** scales with % of map points held. Drives the enemy **spawn table** — WL1–2 militia/trucks → WL3+ regulars/APCs (airfield capture unlocks) → WL5+ tanks, attack-heli QRF, artillery. High WL also: fewer civilians, tougher roadblocks, higher undercover-detection, capped trainable skill.
- **Heat / Aggression (per-region, fast):** spikes on contact/kills/blown cover; **decays if you lie low**. Drives *right-now* response — patrol frequency, QRF size, counterattacks. Each region has an explicit cooldown timer so the world stays conquerable.

Both are scalar globals gating which groups get spawned. Plan [08](./08-ai-shared-enemy-intel.md) (shared intel) makes neighboring garrisons react before visual contact — the "the whole net lit up" feeling — without per-unit cost.

### Liberation must visibly change the world

A captured zone must show consequence: friendly garrison appears, a recruiter/vendor object spawns, it becomes a fast-travel node (`setPos` between owned camps), patrols turn friendly. *"Empty is the absence of consequence, not stuff."*

### Win / lose

- **Win:** own every zone (or every airfield + enemy HQ destroyed). The final liberation is a pitched battle at max War Level, not a stealth job.
- **Lose (soft):** all named companions dead **and** ₽/HR at zero with no owned camp → "rebuild from one camp" fail-state (keeps it a sandbox).
- **Lose (hard, optional ironman):** player permadeath.

---

## Core requirement: swappable factions & islands

**A new game = pick an island + three factions (civilian, resistance, occupier).** This is a launch requirement, not a stretch goal, and it dictates architecture ahead of content:

### Factions are data packs; the side model stays fixed

The player is always the GUER *slot*, the occupier the EAST *slot*, civilians the CIV *slot*. `createUnit` spawns a unit **into a group** (`Game/Commands/GameStateExtWorld.cpp:237`) and group membership determines side, so any faction's classnames can fill any slot — "the occupier is the US Army" is just WEST-flavored classnames spawned into EAST-side groups. (Verify cross-side class spawning once in-game on this port; it worked in retail OFP.)

A **faction descriptor** carries everything scripts are currently tempted to hardcode:

- unit roster by **role × War-Level tier** (rifleman / officer / MG / AT / crew × tier 1..n)
- vehicle pool by tier (truck → APC → tank → heli)
- civilian model set (for the civ faction and undercover disguises)
- starting gear + the loot-classname → unlock mapping

Scripts reference **roles and tiers only, never classnames**. `GM_fnPickTier`-style helpers read the active descriptors chosen at new-game.

### Islands are data + thin shells; the script core is shared

The world is fixed by the mission folder's suffix (engine fact: `MissionPathLoader`), so one mission folder per island is unavoidable — but each shell should contain only `mission.sqm` + a **per-island data file** (military zone list, camp/start position, tuning). All managers live in one shared location, not copy-pasted per island. Town/CITY zones **auto-seed** from the world's own `Names` config via the `worldLocations` command (see the C++ list above); only military zones (airfield, outposts, camp) are hand-tuned per island.

### Sequencing rule

De-hardcode **before** content breadth: prove the loop on the hardcoded Demo first (Gate-Zero + money moment), then do the faction-descriptor/island-data refactor **before** building a second island or faction. Retrofitting swappability after content exists multiplies its cost. Note the Demo dataset (one island, minimal roster) cannot even exercise swapping — this work tests against the full CWA data (four islands, full EAST/WEST/GUER/CIV rosters).

---

## Persistence design

> Single-player first. No JIP, `publicVariable` syncs values not objects, no native MP save — a persistent shared server is **out of scope for v1**.

**Chosen pattern: one persistent mission + `saveGame`.** The whole island is *one* mission; sub-areas cache/reset via script. All meta-state (zone array, ₽/HR, War Level, per-region Heat, unlocks, companion XP/skill/loadout) lives in **global variables**, which serialize with the save via `GGameState` (`World/WorldImpl.cpp:1738`).

| State | Storage | Mechanism |
|---|---|---|
| Zone ownership/support/garrison | global array `ZONES[]` | `saveGame` world state |
| ₽ / HR / War Level / per-region Heat | globals | same |
| Gear unlocks (tally + unlocked set) | global arrays | same |
| Named companions (alive, rank, XP, skill, loadout) | parallel global arrays keyed by name | re-applied on load: `SetRank`/`setSkill`/`addWeapon` |
| Player position / inventory | engine | native `continue.fps` |

**Critical rule:** persist **IDs and class names, not object handles** — object-to-object references break on save/load. Rebuild live objects on load.

**Optional C++ assist (later):** a custom `SerializeClass` writing a parallel `guerrilla.sav` via `GetCampaignSaveDirectory()`, version-gated alongside `WorldSerializeVersion` in `Core/SaveVersion.hpp`, bridged to SQF via `guerrillaSaveState`/`guerrillaLoadState`. This makes persistence bulletproof and is the seam for co-op persistence later — but is **not** required for the SP MVP.

---

## Leveraging the existing mod-plans

The AI plans are pre-scoped C++ enhancements that map directly onto the guerrilla fantasy. The collective arc they enable: *player ambushes patrol → kills officer + suppresses (04, 05) → survivors flee to cover (03) → nearby garrison radio-alerted (08) → reinforcements maneuver → player breaks contact.*

| Plan | Hook | Guerrilla payoff | When |
|---|---|---|---|
| [09](./09-sqf-enable-ai.md) `enableAI` | new command (`GameStateExt`) | un-freeze cached garrison units | **MVP** |
| [10](./10-sqf-known-targets.md) `knownTargets` | new command; reads `AIGroup` TargetList | ambush/alert scripting | **MVP** |
| [04](./04-ai-target-priority.md) target priority | `GetSubjectiveCost()` (`AIGroupImpl.cpp:834`) | kill-the-officer feels lethal | Phase 2 (first) |
| [05](./05-ai-suppression-response.md) suppression | `SetDanger()` (`AIUnitImpl.cpp:2190`) + `Shots.cpp` | fire-and-maneuver | Phase 2 |
| [03](./03-ai-flee-to-cover.md) flee-to-cover | `FindFleePoint()` + exposure map | believable retreat; secondary-ambush bait | Phase 2 |
| [08](./08-ai-shared-enemy-intel.md) shared intel | `AICenter::ReceiveReport` | radio-net escalation (Heat core) | Phase 3 |
| [02](./02-ai-dynamic-morale.md) dynamic morale | `AIGroup` courage | small garrisons rout when outnumbered | Phase 3 |
| [06](./06-ai-memory-decay.md) / [07](./07-ai-stance-concealment-spotting.md) | various | stealth vs pursuit tuning per War Level | polish |
| [12](./12-dev-ai-diagnostics-tab.md) AI Diag tab | ImGui dev panel | invaluable for tuning spawns/alert during dev | dev-time |

Documented gaps the plans do **not** cover (faked in SQF for v1): native garrison/cover-hold logic, dynamic patrol-route generation, ambush-hold-fire FSM states, supply lines.

---

## Phased roadmap

### Gate-Zero — integrated smoke test (BEFORE any content work)

The original instinct was "persistence is the top risk." It isn't — `GGameState` already serializes. The genuinely **unverified, gating** assumptions are different. In one sitting, on the real build:

1. Build `win-x64-clang-rwdi`, acquire the free Steam Demo data, confirm a **stock mission renders and simulates AI at framerate** on this modernized C++20 port.
2. In one SQF test, runtime-spawn ~6–8 AI in two groups (`createGroup`/`createUnit`), drive a QRF with `doMove`, watch **framerate and pathing** on a real map.
3. Stuff a few zone/roster globals into plain arrays, `saveGame`, quit, reload, and **diff them** (test the cheap path before building any `SerializeClass`).
4. Re-confirm the live command tables in `GameStateExt*.cpp` (already verified: dynamic markers/triggers/groups present).

**Why first:** this validates the three things the whole roadmap silently assumes — the port runs missions, the spawn loop holds framerate within the 12/group budget, and global state round-trips a save. If the port can't run a populated mission at framerate, everything else is blocked.

### Phase 1 — MVP: "Three Zones" (pure SQF, no new art, no required C++)

One corner of an existing map (~2 km²), three zones: **Camp** (player start; recruiter action-menu, stash, fast-travel anchor), **Village** (support-based → HR + recruits), **Outpost** (occupier-held; ~8 men in two groups to respect the 12-cap, one officer).

Ships: graduated alert state machine; outpost garrison on sentry/guard waypoints; one QRF that spawns + convoys on RED (road-snap fallback); loot-on-kill stash tally; `ZONES[]` of 3 + capture triggers + marker recolor on flip; ₽/HR counters on a ~10-min tick; recruit-via-action-menu; **one** named companion with XP→rank/skill; **one** gear unlock (loot 25 AK mags → AK unlocked); War Level with **two** spawn tiers; per-region Heat with decay. All in globals; `saveGame`/`loadGame` round-trip verified. Land plans **09 + 10** (cheap).

*Riskiest unknown:* spawn/cache + convoy loop staying in budget and pathing reliably. *Mitigate:* road-snap spawns, heli/spawn-on-arrival fallback.

### Phase 1.5 — De-hardcoding: swappable factions & islands (BEFORE content breadth)

The refactor the core requirement demands, done while there is only one island and one faction pair to migrate: faction descriptors (roles × tiers, vehicle pools, civ models, unlock mapping) replacing every inline classname and side string; per-island data files + shared script core replacing the fused Demo mission; the evaluator QoL commands and `worldLocations` landing engine-side so the refactored scripts are written against the real API, not the Phase-1 workarounds; and a minimal new-game selection flow (wizard-derived screen, or the `addAction` setup-menu stopgap). Exit criterion: **the same script core runs on a second island with a different occupier faction, selected at game start.** Tests against full CWA data, not the Demo.

### Phase 2 — Vertical slice: economy + growth + persistence

Multi-zone island; full meta-loop (₽/HR ticking from owned zones, recruitment, XP/training, gear-unlock arsenal, War Level + Heat, undercover). Productize persistence. Action-menu command interface (no full UI). AI fidelity: plans **04, 05, 03**.

*Riskiest unknown:* avoiding the empty-sandbox/grind traps. *De-risk:* prove one captured zone visibly changes the world; gate progression on **capabilities, not stat inflation**.

### Phase 3 — Full mode: strategic UI + escalation depth + nemesis

`GameModuleId::Guerrilla` registered; `GuerrillaDisplay` strategy screen on `UIMap` (recruit/train/assign/spend, mission requests, zone status); shared-intel coordination (plan **08**); dynamic morale (**02**); formations/memory/stance (**01/06/07**) polish; scripted nemesis-lite (named commanders persisted with buffed skill/loadouts); per-region Heat cooldown.

*Riskiest unknown:* C++ UI scope creep. *De-risk:* read-only `UIMap` marker-overlay prototype before any interactive `Display` work; the game must be fun without the strategy screen.

### The MVP "money moment" (the acceptance test for the whole game)

> Ambush the outpost patrol → drop the officer (group routs) → loot 25 mags → AK unlocks faction-wide → recruit two fighters with banked HR → your veteran rank-ups → assault and **flip the Outpost** → marker turns friendly, income starts, **War Level ticks up** → the next patrol that wanders by is a *regular* squad with an APC → **save, quit, reload — veteran, unlock, and territory all persist.**

If that ~15-minute sequence is fun and persists correctly, the whole game is proven. Everything past it is **content breadth** + **C++ fidelity**, not new core systems.

---

## Packaging & shipping

**This project ships as the fork.** Upstream is locked (no PRs accepted), so the overhaul is the patched engine binary + content PBOs together — there is no "unmodified engine" deployment target to protect. The layering below still applies because it keeps content iterable without rebuilds, not because engine deltas are to be minimized.

1. **Mission/campaign PBO** — `mission.sqm` + SQF (`init.sqf`/`init.sqs`, loaded by `RunInitScript()` in `UI/DisplayUI.cpp`); zone/economy/spawn managers. Packaged with the **TcPbo** tool (`apps/tools/TcPbo`).
2. **Asset addon PBO(s)** in `AddOns/` — reuse stock `CfgVehicles` GUER/EAST/WEST units; new models only if needed.
3. **Engine deltas** fold into `Poseidon.lib` and the existing `Game` client via CMake/Clang (per `CLAUDE.md`): the optional persistence `SerializeClass` + `SaveVersion.hpp` constant, new SQF commands in `GameStateExt*.cpp` (`INIT_MODULE` at `:1468`), `GuerrillaDisplay` + `GameModuleRegistry` registration, adopted mod-plans. **Not** a separate app target — `GameModuleRegistry` gives a first-class main-menu mode with zero `GameBase` duplication.
4. **Distribution:** PBOs into `Missions/`/campaign dir; engine deltas in the patched binary. Validate headless via Trident.

---

## Risks & gotchas

| Risk | Severity | Mitigation |
|---|---|---|
| **Port runtime playability unverified** — "locked" release, CI compiles tests only, ~25 unit tests fail OOTB, integration tests need separate data | High | **Gate-Zero** smoke test before anything else |
| **Engine scale** — 12/group, ~63/side, single-threaded (~50–100 live AI, *unmeasured on this port*) | High | aggressive distance-caching; HR cap as governor; **profile actual AI-vs-framerate early** |
| **AI-feel plans 02–05/08 are real C++ surgery, not polish** — the core "asymmetry you feel" depends on them | Medium | treat as a funded Phase-2/3 milestone; prototype 04 first and measure |
| **Cache correctness across save/load** — a named companion's identity/XP/permadeath must survive despawn→save→reload→respawn | Medium | cache is authoritative state machine: despawned = integer strength, spawned = transient view; test the full cycle in Phase 1 |
| **Convoy/QRF pathfinding** historically unreliable | Medium | road-snap spawns; heli/spawn-on-arrival fallback so escalation always shows up |
| **Empty-sandbox / grind** | Medium | captured zones must visibly change; gate on capabilities not stats |
| **`addAction` UI is fiddly; `Display` UI balloons** | Low | keep UI out of MVP; read-only `UIMap` overlay before interactive UI |
| **Persistence edge cases** | Low–Med | persist IDs/classnames never handles; explicitly test player-dead and mid-SQS-loop saves; build `SerializeClass` only if globals don't round-trip |

---

## Testing strategy

- **Gate-Zero** (above) is the first and most important test — it gates the project.
- **Trident integration** (`tests/integration/scripting/`): per-system SQF scenarios — spawn-and-command a group, capture-trigger flip, save/reload state round-trip, undercover break detection. Mirror existing `.test.sqf`/`.toml` pairs; run with `tri test -j6 --retries 2 tests/integration` after `cargo build --manifest-path engine/Trident/Cargo.toml`.
- **Catch2 unit tests** for any new C++ command (e.g. plan 09's `enableAI` mask round-trip), suite `PoseidonTests`/`PoseidonServerTests`.
- **In-game smoke** on `win-x64-clang-rwdi`: play the "money moment" end-to-end; eyeball framerate with the plan-12 AI Diag tab.

---

## Scope estimate

- **Gate-Zero:** ~1 day (build + Demo data + four checks).
- **Phase 1 MVP (pure SQF):** ~1–3 weeks of mission scripting; **no engine build required** (plans 09/10 optional but cheap, ~1–2 h each).
- **Phase 1.5 (de-hardcoding):** ~1–2 weeks — faction descriptors + island data split (script refactor) + the evaluator QoL / `worldLocations` commands (each in the plans-09/10 cost class) + minimal new-game selection.
- **Phase 2:** several weeks SQF + plans 04/05/03 (each intermediate–advanced C++; see their scope estimates).
- **Phase 3:** open-ended; the `GuerrillaDisplay` UI is the largest single C++ surface.

**Minimal first slice for a visible win:** the Gate-Zero spawn-and-`doMove` test *is* a playable proof — six fighters, one QRF, on a real map. From there, the "Three Zones" MVP is the smallest thing that demonstrates the entire loop.

---

## One-line summary

*Resistance's men-and-gear persistence + MFCTI's town economy + DAC's cached spawning, fused into a War-Level-escalating insurgency where you ambush, loot, recruit, train, and liberate an island zone by zone — on **any island, against any occupier, as any resistance faction, chosen at new-game** — built on an overhauled CWA engine (evaluator upgrades, world-data commands, new-game flow) plus its reactive tactical AI, deliberately scoped under the engine's hard unit caps.*
