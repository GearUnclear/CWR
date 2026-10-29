# Open Poseidon: roadmap, issues and community

Researched 2026-10-29 via the web and GitHub API.

## Where it lives

- Home: https://github.com/koosoli/OpenPoseidon. Created 2026-10-05, last push 2026-10-25, 16 stars, 6 forks, 27 commits, 0 open issues, no releases, no milestones, no pinned issues; wiki and projects enabled but empty; discussions off.
- Predecessors by the same owner:
  - https://github.com/koosoli/OpenPoseidon_legacy: formerly "PoseidonWGPU", fork of paavohuhtala/CWR-CE, created 2026-07-21, last push 2026-10-27, 5 stars. Its August 2026 history shows wgpu made the default renderer (2026-08-06), grass, FFT water, cloud shadows, stars, GTAO, Zeus mode, and a "Preview 0" packaging runbook.
  - https://github.com/koosoli/PoseidonVK: Vulkan backend work on CWR-CE, created 2026-07-04, last push 2026-07-20, 12 stars. Phased checklist: Phase 0 backend-neutral core (done), Phase 1 minimal Vulkan (done), Phase 2 Vulkan raster parity (in progress). Appears superseded by the wgpu route.
- Org https://github.com/OpenPoseidon (created 2026-10-07, no public repos) hosts the private `OpenPoseidon-Builds` repo for sponsors.

## Roadmap

There is no formal roadmap, milestone, project board or pinned issue. The only planning artifacts are the README's "Current limitations" section, "Planned" rows in the Proteus fork's README, and the PoseidonVK phase checklist. Everything under "planned" below is inferred, not announced.

## The whole tracker (3 items)

- [#1](https://github.com/koosoli/OpenPoseidon/issues/1) issue, closed 2026-10-08, "Community platform": octane038 proposed a Discord channel and offered Blender tooling for Arma Reforger content; the owner moved discussion to Discord (channel `#olik-fork`).
- [#2](https://github.com/koosoli/OpenPoseidon/pull/2) PR, merged 2026-10-15 (jschmidt92): build precompiled CLI11 in C++20 mode to fix a startup crash.
- [#3](https://github.com/koosoli/OpenPoseidon/pull/3) PR, merged 2026-10-15 (jschmidt92): 21 in-memory database commands plus `getUnitLoadout`/`setUnitLoadout`, `PDB1` binary save format, ported from cwr_ce commits `ed938b1` and `04145d3`. Claimed validation: 645 assertions and one wgpu single-player smoke mission; multiplayer and POSIX not validated.

No feature requests have been filed. The project is driven by the owner's commits.

## Feature status by area

From the README as of the 2026-10-25 commit.

**Shipped in source** (self-described; "features are at different stages of acceptance"):

- Graphics: wgpu renderer (Vulkan on Windows and Linux, D3D12 on Windows); GTAO and bent-normal ambient; probe-grid GI; dynamic sun and local shadows, contact shadows; clear-coat paint with SSR; volumetric clouds, procedural sky, stars, cloud shadows; layered volumetric fog; GPU-driven trees; FSR 1 and optional local DLSS; procedural grass, puddles, snow depth and tracks, mud ruts.
- FX and destruction: volumetric fire and smoke with portal-aware containment, impact craters, bullet holes in walls, destructible terrain (deformation, caves, trenches), muzzle flash lighting.
- Water and environment: FFT Tidewater ocean, caustics, underwater rendering, floating boats, fish schools, coastline detail.
- Physics and animation: Box3D rigid bodies and ragdolls (37 stock models tested), reload while walking, Q/E lean, jump, swim, dive, firing from vehicles (stock Jeep).
- AI and gameplay: gunfire hearing, suppression, search, grenade avoidance, optional gradual spotting and flanking (config-dependent); single-player physical inventory (Tab); experimental LLM NPC chat.
- Audio: Steam Audio default (HRTF, occlusion, reverb, underwater), OpenAL fallback, coastal soundscape.
- Worlds and content: experimental Arma 1/2/3 mission loading; Arma 1/2/3, DayZ and Reforger terrains in place; whole-Earth streaming plus Moon (needs internet); Island Editor; OFP Fusion archipelago; local content browser.
- Scripting: in-memory database and loadout commands.
- Tools: dev panel, Zeus free-fly and unit placement, GPU timings, enkiTS multithreading for some jobs.
- UI: optional modern main and options menus via `assets\Install-Menus.cmd`, copied from the free CWA Remastered Demo; classic menus otherwise.

**In progress** (limitations the README admits): fog patterning; two Camel pilot ragdoll settling issues; ragdoll admission limits (posture, saved corpses, network ownership, budgets); weather materials and modern-game material/LOD conversion; soft ground (no general soft-body vehicles); Arma 1/2/3 missions stopping at missing script commands or content.

**Planned** (inferred only): the Proteus fork lists a Multiplayer screen and a Mission Editor launcher restyle as planned; Proteus issue #1 is about a mod-path and install-folder claim protocol.

**Requested:** nothing in the tracker beyond the Discord and modding-tools offer in #1.

## Downstream forks (all October 2026)

- **Proteus**, https://github.com/theantipopau/OpenPoseidon (last push 2026-10-27, 1 open issue): restyled main menu, options, campaign, single missions, editor dialogs, debriefing and unit info; DX12 fixed on AMD RDNA4; FSR 4.1.1 working with user-supplied AMD DLLs; 3D models drawn over the tonemapped UI. Removed the Mods menu and Local Maps (Local Maps crashes; Mods has "no working route yet"). Says gameplay and mission scripting are unchanged and the new screens have not been tested through a full mission.
- octane038/OpenPoseidon (mirror plus Oct 25 updates), jschmidt92/OpenPoseidon (PR author), Dahlgren/OpenPoseidon, steffenbk/OpenPoseidon-BK, seancombsx/WW2 ("WW2 Based OPEN Poseidon Fork"). Mostly unmodified.

## Community footprint

- Searches of Reddit, the ofpisnotdead.com forum, BI forums, ModDB and YouTube found nothing about Open Poseidon.
- The only community channel is a Discord, which was not joined or read.
- Public footprint is the GitHub repo alone.
- Context: Bohemia's own release was covered by GamingOnLinux, PC Gamer and KitGuru. The official repo is closed to PRs and not continuously updated, which is why forks like CWR-CE exist.
- Unrelated Vulkan experiment in the same space: https://github.com/oksklok/cwr-vulkan (based on official 3.05).

## Compatibility facts and limitations

- Needs your own CWA (Steam or GOG) or OFP; the binaries unpack next to the original exe and overwrite nothing.
- OFP/CWA content is "the primary gameplay target", but COMPATIBILITY.md says one successful test does not imply support for every mod or savegame. No specific OFP mods are named as supported. Mod support is unproven.
- Imported modern terrains are not ports of those games' simulation, missions or configs; vehicle compatibility is selective.
- Multiplayer: no claim of compatibility with stock CWA 1.99 or Remaster clients or servers. MP keeps classic Gear; Dev Tools and Zeus are disabled; firing from vehicles is limited; ragdoll network ownership is a limit. With new rendering, physics, animation and scripting, MP interop should be treated as unverified and probably divergent.
- Heavily AI-assisted development; the legacy README says funds go to "API usage" and "AI-assisted code exploration". Validation is mostly local with proprietary content.
- Prebuilt builds are sponsor-only; GPL source is the public route. Whole-world mode needs internet. DLSS is disabled in distributable builds.
- Very young as a public project (source published 2026-10-07, private development since July) with one main developer.

## Compared with CWR-CE and Bohemia CWR

- **Bohemia CWR:** official, GPL-3.0+, C++20, CMake, Clang, Windows x64 and Linux x64; closed to PRs, not continuously updated.
- **CWR-CE:** 81 stars, 33 forks, active community project with a CI builds page; Windows, Linux and macOS (arm64 and x64). Many merged PRs through 2026-10-07: instanced terrain rendering and object batching perf ports, mods-menu and MP-room fixes, config tolerance, Linux hardening, bindable chat controls, widescreen cursor fix, warnings-as-errors. Focus: stability, performance, platform breadth, upstreaming. Keeps the stock OpenGL renderer.
- **Open Poseidon adds** the wgpu/Vulkan/D3D12 renderer with GI, GTAO and volumetrics; ocean, weather, snow and mud; destructible terrain and craters; Box3D physics and ragdolls; firing from vehicles; new animations; Steam Audio; optional AI behaviours; LLM NPC chat; single-player physical inventory; Arma 1/2/3 mission and DayZ/Reforger/Arma terrain loading; whole-Earth and Moon rendering; Island Editor; database and loadout scripting (ported from cwr_ce).
- **Open Poseidon lacks** CWR-CE's platform breadth, public CI builds, open contribution volume and any statement on stock multiplayer compatibility. It is a one-person, feature-first fork that cherry-picks from CWR-CE (PR #3), not the reverse.
