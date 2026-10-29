# Open Poseidon vs Uslu dur! (research, 2026-10-29)

What Open Poseidon is, its full claimed feature set, and how it compares to UD.

**Method.** Three research agents covered Open Poseidon from separate angles (identity and docs, the source tree, roadmap/issues/community). The UD side was inventoried from `roadmap/roadmap.yaml`, the engine tree and the branch list. Nothing from Open Poseidon was built or run: every Open Poseidon feature here is what its README claims and what its source tree contains, not observed behaviour.

Detailed reports:

- [identity-and-docs.md](identity-and-docs.md): lineage, license, data requirements, the README feature list item by item
- [source-code.md](source-code.md): layout, build matrix, subsystem-by-subsystem code evidence, diff against CWR-CE
- [roadmap-and-community.md](roadmap-and-community.md): issue tracker, downstream forks, community footprint, compatibility facts

## What it is

[koosoli/OpenPoseidon](https://github.com/koosoli/OpenPoseidon) ("Open Poseidon Engine") is a modernization fork of Bohemia's Poseidon engine source.

- Lineage: BohemiaInteractive/CWR, then ofpisnotdead-com/CWR-CE, then paavohuhtala/CWR-CE, then koosoli's `OpenPoseidon_legacy` (formerly "PoseidonWGPU"), then Open Poseidon.
- Open Poseidon itself is a squashed export, not a GitHub fork. The first commit is "Open Poseidon Engine source release" (2026-10-07). Earlier history lives in [koosoli/OpenPoseidon_legacy](https://github.com/koosoli/OpenPoseidon_legacy).
- License: GPL-3.0-or-later with Bohemia's Section 7 terms, the same as ours.
- Scale as of 2026-10-29: 27 commits, 2 contributors (koosoli as "Deus-Ex", and jschmidt92), 16 stars, 6 forks, no CI, no public releases (one tag, `v0.2`).
- Prebuilt Windows builds go only to GitHub Sponsors via a private repo. Building from source is free.
- Development is openly AI-assisted. Most features landed in one bulk commit on 2026-10-25, so git history cannot show how mature each one is. The README says features "are at different stages of acceptance".

## Side by side

| Area | Open Poseidon | Uslu dur! |
|---|---|---|
| **Direction** | Engine showcase: graphics, physics, importing content from other games | Gameplay overhaul: Guerrilla Mode open-world insurgency, AI, controls |
| **Base data** | Your own CWA or OFP install (Classic-era layout); Remaster Demo only for its modern menus | Full Remaster; Classic support being dropped |
| **Renderer** | New Rust wgpu renderer (Vulkan, D3D12), about 220k lines; GL33 kept as legacy | GL33. Lighting, fog and shadow rework on the `graphics-remaster` branch (113 commits ahead of main, not merged) |
| **Lighting / post** | GTAO, probe-grid GI, SSR, clear-coat vehicle paint, volumetric clouds, layered volumetric fog, procedural sky, FSR 1, optional local DLSS | Remastered fog, shadow offset, night lights, cockpit fog, AI sight tied to the fog actually drawn (branch only) |
| **Water / weather** | FFT ocean (Tidewater port), caustics, underwater rendering, boats floating on drawn waves, puddles, snow depth and tracks, mud ruts | Stock |
| **FX / destruction** | Volumetric fire and smoke with room/portal containment, craters, bullet holes in walls, deformable terrain, caves, trenches | Stock |
| **Physics** | Box3D rigid bodies and articulated ragdolls | Stock |
| **Audio** | Steam Audio default (HRTF, occlusion, reverb, underwater), OpenAL fallback, coastal soundscape | OpenAL; TTS voice-line pipeline for content |
| **Infantry** | Q/E lean, jump, swim, dive, reload while walking, firing from vehicles (stock Jeep validated) | Q/E rigid 35 degree lean moving camera, weapon, AI target points and damage geometry, with swept cover checks (single-player only, #68) |
| **Controls** | SDL3 input and gamepad UI (both trees inherit this) | Arma 3 RMB zoom/sights, direct-look mouse, no mouse steering in vehicles, gamepad steering option; Arma 3 aircraft keyboard controls planned |
| **AI** | Optional, config-dependent: gunfire hearing, near-miss suppression, search where danger came from, grenade avoidance, gradual spotting (mostly ported from the Malprave fork) | Per-observer deep undercover perception, lean-aware targeting. AI tactics tranche (target priority, suppression response, flee to cover, morale) still planned |
| **Inventory** | Physical drag-and-drop inventory with weight and volume budgets (single-player; MP keeps classic Gear) | Stash registry, HQ cache, vehicle garage, dealers (economy side, no personal inventory UI) |
| **Game mode** | None; plays existing missions | **Guerrilla Mode**: zones, garrisons, alerts, persistence, swappable islands and factions, HQ/base, market, traffic, civilians, extortion, unassociated assailants, undercover, occupier director (retakes, waves, air, HQ loss, lose state), Legends, field journal/dossier, character select |
| **Content import** | Arma 1/2/3 missions (experimental, Arma 1 furthest along); Arma 1/2/3, DayZ and Reforger terrains read in place; whole-Earth streaming plus Moon (needs internet); "OFP Fusion" archipelago; Island Editor | Third-party OFP mod intake (`PoseidonTools mod doctor`, faction packs, island packs), @LoBo repairs, 4WVR world-loader fix (upstreamed to CWR-CE #98) |
| **Scripting** | About 692 commands; about 274 added over CWR-CE, mostly Arma 3-style (`params`, hash maps, `execVM`, `ctrl*`/`lnb*` UI), plus in-memory DB (`db*`) and `get/setUnitLoadout` | About 531 commands, including about 157 native `gm*` Guerrilla services, `nearestRoads`, `worldLocations`, stash cargo commands |
| **LLM features** | NPC chat with squad, civilians and HQ (OpenAI, Anthropic, Gemini, OpenRouter, Ollama; STT and TTS); orders map to game commands, model output never runs as script | None |
| **Dev tools** | Zeus free-fly unit placement, GPU timings, dev overlay, `--diag` event log, ReShade bridge, texture workbench | Trident headless test harness; PoseidonTools Guerrilla lint/probe/scaffold and mod doctor; screenshot shoot scripts |
| **Testing / CI** | Catch2 suites, but binary fixtures omitted so the asset-dependent suite cannot pass; no CI (CWR-CE's workflows were dropped) | CI on unit suites and mod-intake lanes; red/green crash regressions; Trident integration lanes |
| **Multiplayer** | No new netcode; Zeus, dev tools, physical inventory and ragdoll network ownership off or limited in MP | No new netcode; lean disabled in MP pending pose replication |
| **Platforms** | Windows wgpu primary; Linux, Steam Runtime 4 and macOS presets "not a promise of equivalent runtime support"; Linux arm64 dropped | Windows x64 primary; Linux preset inherited |

Command counts come from the same `GameFunction(..."name")` regex run on each tree (CWR-CE scores 409). Macro-registered commands are missed, so all three are lower bounds.

## Overlaps

- **Lean.** Both have Q/E lean. Ours carries hit geometry, AI target points and cover checks with the pose. Theirs is documented as an animation feature.
- **AI perception.** Their hearing, suppression and search behaviour covers ground close to our planned AI tactics tranche (mod-plans 03, 04, 05).
- **Fog.** Both reworked fog. Ours also ties AI sight to the rendered fog; theirs lists "dense/aerial fog can still show patterns" as a known limitation.

## Only in Open Poseidon

Modern renderer stack (wgpu, GI, SSR, volumetrics, ocean), Box3D physics and ragdolls, Steam Audio, swimming and diving, physical inventory, firing from vehicles, destructible terrain, Arma 3-style script commands, Arma 1/2/3 and Reforger content loading, whole-Earth rendering, LLM NPC chat.

## Only in Uslu dur!

An actual game mode (all of Guerrilla), the occupier AI director, economy, traffic, civilians, undercover, Legends and the journal; full Remaster targeting; mod-intake tooling; a real CI and regression-test culture.

## Borrowing notes

Both projects are GPL-3.0+ with the same Section 7 terms, so code can be ported with credit. Exceptions and caveats:

- DLSS sits behind a Cargo feature because the NGX SDK is proprietary and GPL-incompatible; Open Poseidon itself ships builds without it.
- Tidewater (ocean) and ez-tree content are MIT.
- Much of their AI and swimming work is credited to the separate "Malprave" GPL fork, which may be the cleaner source to port from.
- Their Arma 3 script commands carry a header saying they were written from public command descriptions with no Bohemia code.

Best fits for our direction: swimming, the AI hearing/suppression/search pieces (as inputs to mod-plans 03-05), and possibly Steam Audio occlusion. The wgpu renderer is a separate 220k-line Rust subsystem; adopting it is an architecture decision, not a port.

## Sources

- https://github.com/koosoli/OpenPoseidon (README, CREDITS.md, COMPATIBILITY.md, BUILDING.md, ASSETS.md, issues/1, pull/2, pull/3)
- https://github.com/koosoli/OpenPoseidon_legacy
- https://github.com/koosoli/PoseidonVK
- https://github.com/theantipopau/OpenPoseidon (the "Proteus" downstream fork)
- https://github.com/ofpisnotdead-com/CWR-CE
- https://github.com/paavohuhtala/CWR-CE
- https://github.com/BohemiaInteractive/CWR
