# Mod Plans — Poseidon / Arma: Cold War Assault

This is the engine/AI layer of a **total game overhaul** built on top of Arma: Cold War Assault (Poseidon): a from-the-ground-up rework of how the AI fights, perceives, and is commanded, culminating in [Guerrilla Mode](./13-guerrilla-mode.md) — a whole new persistent open-world game mode. Every plan here is implementable **without any art assets** — no models, textures, sounds, or missions. They touch only engine/game C++ and the SQF scripting surface.

Most plans target the engine's **AI** (tactics and perception), which is the recommended primary focus: it is the richest, most self-contained area to mod and most plans reuse the same handful of subsystems.

## All plans

| Plan | Title | Area | Difficulty | What it does |
| --- | --- | --- | --- | --- |
| [02](./02-ai-dynamic-morale.md) | Dynamic morale | ai-tactics | intermediate | Turns frozen leader-skill courage into a live morale value that decays with casualties, craters when the leader is wounded, and rises with nearby friendlies. |
| [03](./03-ai-flee-to-cover.md) | Smarter retreat: flee toward cover | ai-tactics | intermediate | Rewrites `FindFleePoint()` to sample low-exposure points away from the nearest enemy so routed squads break toward cover instead of sprinting to spawn. |
| [04](./04-ai-target-priority.md) | Target-priority retuning | ai-tactics | intermediate | Reshapes `AIGroup::GetSubjectiveCost()` so groups focus-fire enemy leaders, MG/crew gunners, and nearer/wounded targets, with optional cross-group de-confliction. |
| [05](./05-ai-suppression-response.md) | Working suppression | ai-tactics | advanced | Unlocks the engine's danger-window reaction in `CMCombat` and feeds it from a per-bullet near-miss test so passing rounds force AI prone and into cover. |
| [06](./06-ai-memory-decay.md) | Tunable AI memory decay | ai-perception | beginner | Replaces three hardcoded `Target.cpp` forgetting curves with a cached config struct, tuning AI from 10-minute memory to instant amnesia; verifiable via `knowsAbout`. |
| [07](./07-ai-stance-concealment-spotting.md) | Stance- & concealment-weighted spotting | ai-perception | intermediate | Re-weights the `visibleSize` stance term and `GetHidden()` cover term so a still, prone, in-cover soldier is far harder to detect at range. |
| [08](./08-ai-shared-enemy-intel.md) | Cross-group shared enemy intel | ai-perception | advanced | Extends `AICenter::ReceiveReport` so a fresh contact seeds nearby friendly groups with a degraded copy — squads react to a teammate's radio call before LOS. |
| [09](./09-sqf-enable-ai.md) | `enableAI` / `enableAIFeature` | scripting | beginner | Adds a mirror `enableAI` operator that clears the AI-disable bitmask `disableAI` only ever sets, so scripts can un-freeze MOVE/TARGET/AUTOTARGET/ANIM at runtime. |
| [10](./10-sqf-known-targets.md) | `knownTargets` | scripting | intermediate | Adds a unary SQF command returning a group's known contacts as `[object, side, knowledge, lastReportedPos]`, exposing AI perception in bulk. |
| [12](./12-dev-ai-diagnostics-tab.md) | AI Diagnostics tab | dev-qol | intermediate | Adds an "AI Diag" tab to the ImGui dev panel exposing the engine's existing steering/path/cost-map/lock-map overlays as one-click checkboxes. |
| [13](./13-guerrilla-mode.md) | Guerrilla mode (open-world insurgency) | game-mode | project | Master plan for a persistent open-world guerrilla game — grow & train a faction, capture occupied towns, escalating enemy, on any island against any occupier chosen at new-game. Mostly SQF over the engine, with engine-side work (faction/island data, evaluator commands, new-game flow) as a first-class part of the design, not a last resort; consumes plans 01–10 as dependencies. |
| [14](./14-occupation-systems.md) | Occupation systems (COIN world-gen) | game-mode | intermediate | Breakout from 13: the occupier as a population-control state — checkpoints/curfews as generated terrain, closures/reprisals/resettlement as moves on the player's support economy, informers, pseudo-gangs, Fireforce QRF, detention/rescue. Historical COIN doctrine as systems; one new road-query command, zero new animations. |

## Recommended focus: AI

Plans **02–08** are the AI core (tactics + perception) and are the recommended primary focus. Plans 09–10 (scripting) double as **probes/test hooks** for the AI work, and 12 (dev tooling) supports and validates it. Plan **13** is the umbrella **game-mode** plan — a persistent open-world guerrilla insurgency that *consumes* the AI plans (02–05/08) and scripting commands (09/10) as dependencies; read it last, as the application that ties the engine work to a game.

## Suggested learning path

Start gentle, build up. Each step leans on what you saw in the previous ones.

1. **[12] AI Diagnostics tab** — pure UI wiring onto an existing draw subsystem. Do this first so you can *see* the AI: live path/force/combat overlays validate everything else.
2. **[06] Memory decay** — three fade curves moved to config, verifiable via `knowsAbout`. Learn the config plumbing.
3. **[09] `enableAI`** — a near-copy of `disableAI`. Learn the command-registration pattern.
4. **[10] `knownTargets`** — learn how engine state is marshalled back to SQF. (09 + 10 give you scripting probes for the harder work.)
5. **[02] Dynamic morale** and **[04] Target priority** — single scoring functions, localized intermediate behavior.
6. **[03] Flee to cover** and **[07] Stance/concealment spotting** — reuse the exposure-cost and visibility machinery from above.
7. **[05] Working suppression** (flagship) and **[08] Shared enemy intel** — capstones that tie multiple subsystems together. The diagnostics tab and the `knowsAbout`/`knownTargets` probes feed directly into building and verifying these.

## Build & test

```sh
cmake --preset win-x64-clang-rwdi
cmake --build build/win-x64-clang-rwdi
ctest --test-dir build/win-x64-clang-rwdi --output-on-failure
```
