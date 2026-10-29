# Open Poseidon: identity and official docs

Researched 2026-10-29 via the GitHub API and web search. Nothing was built or run. "Shipped" below means the repo claims it on `main`; it is not verified behaviour.

Source shorthand: README, COMPAT, BUILD, CREDITS, ASSETS and APPS refer to `README.md`, `COMPATIBILITY.md`, `BUILDING.md`, `CREDITS.md`, `ASSETS.md` and `apps/README.md` in https://github.com/koosoli/OpenPoseidon.

## Identity

**Canonical home:** https://github.com/koosoli/OpenPoseidon ("Open Poseidon Engine"). No separate website, wiki or GitHub Releases page. Web searches for "Open Poseidon" returned only press coverage of Bohemia's own CWR release.

**Unrelated projects with the same name:** `pachterlab/poseidon` (syringe pumps), `poseidon/typhoon` (Kubernetes), `faucetsdn/poseidon` (network classification), Poseidon Linux, various zk-hash repos.

**What it is:** a community modernization fork of Bohemia's Poseidon engine (the CWR source release), not a clean-room reimplementation. The repo description: "a modernization-focused fork of the classic Poseidon engine behind Arma: Cold War Assault / Operation Flashpoint: Cold War Crisis". The README says it "requires your own game data and is not a complete game or an official Bohemia Interactive product."

**Maintainer and contributors:**

- GitHub user `koosoli` (display name "Deus-Ex", account created 2014-06-06). Commits are authored as "Deus-Ex".
- `jschmidt92` contributed 13 commits: CWR-CE in-memory database and unit-loadout commands, plus a CLI11 C++20 startup crash fix.
- A GitHub org `OpenPoseidon` (created 2026-10-07, no public repos) hosts the private `OpenPoseidon-Builds` repo.
- Funding: GitHub Sponsors and Buy Me a Coffee. The README says AI-assisted development consumes API tokens.

**Lineage (CREDITS.md and sibling READMEs):**

| Repo | Created | Last push | Notes |
|---|---|---|---|
| BohemiaInteractive/CWR | 2026-06-22 | 2026-08-18 | 1226 stars, 185 forks; closed to PRs |
| ofpisnotdead-com/CWR-CE | 2026-06-22 | 2026-10-07 | 81 stars, 33 forks; "Retro and its contributors" |
| paavohuhtala/CWR-CE | 2026-06-29 | 2026-08-24 | fork of CWR-CE |
| koosoli/OpenPoseidon_legacy | 2026-07-21 | 2026-10-27 | fork of paavohuhtala/CWR-CE, formerly "PoseidonWGPU"; wgpu became the default renderer 2026-08-06; full dev history |
| koosoli/OpenPoseidon | 2026-10-05 | 2026-10-25 | squashed, cleaned source release; 27 commits, 16 stars, 6 forks |

Other code credited in CREDITS.md:

- `jschmidt92/cwr_ce_source`: database and loadout commands.
- "Malprave", a separate GPL fork of the same Poseidon source with no shared history: DDS formats, terrain holes, swimming, AI fixes, diagnostics.
- Tidewater (`dgreenheck/tidewater`, MIT): ocean, shore waves, foam, spray, breakers.
- GodotOceanWaves (reference) and OpenXRay (behaviour ideas only).

Sibling project `koosoli/PoseidonVK` (created 2026-07-04, last push 2026-07-20, 12 stars) is a separate Vulkan-focused fork of CWR-CE with a hand-written Vulkan backend. It is not Open Poseidon and appears superseded by the wgpu route.

**License:** GPL-3.0-or-later with Bohemia's Section 7 additional terms: no Bohemia or Operation Flashpoint trademark use or implied affiliation, modified versions must be marked, indemnification of Bohemia when assuming contractual liability. Game data stays under APL-SA. GitHub reports the license as "Other" because of the extra terms.

**Releases:** one tag, `v0.2`. No GitHub Release object, no changelog. Ready-made Windows builds only via the sponsor-only private repo.

**Data requirements (README, ASSETS, COMPAT):**

- Your own Arma: Cold War Assault (Steam or GOG) or Operation Flashpoint. Unpack `OpenPoseidon.exe`, `wgpu_renderer.dll` and `assets/` into the game folder; the original game still starts and nothing is overwritten.
- Optional: the free CWA Remastered Demo on Steam, then `assets\Install-Menus.cmd` copies its modern main and options menus. Without it the classic menus are used.
- Arma 1/2/3, DayZ and Reforger content needs those games installed; their files load in place.
- No retail data bundled. Binary test fixtures were deliberately omitted, so the asset-dependent test suite cannot fully pass from the repo alone.

## Feature list

### Platforms, build and renderer backends

- Vulkan through Rust wgpu on Windows and Linux, plus Direct3D 12 on Windows. The active API depends on platform, GPU and driver. [README] Shipped.
- Windows wgpu is "the primary development target". Linux and macOS CMake presets exist but are "not a promise of equivalent runtime support". [README] Partial.
- The legacy OpenGL 3.3 renderer (`engine/PoseidonGL33`) is still in the tree but no longer mentioned in the README. Legacy.
- Build: CMake 3.25+, Ninja, clang-cl/LLVM, vcpkg, stable Rust, Python with Pillow and NumPy. `scripts/Build.ps1 -Preset win-x64-clang-rwdi -Target PoseidonGame`, output in `dist/x64-win-rwdi`. [BUILD]
- Upscaling: AMD FSR 1 shipped. NVIDIA DLSS is an optional local integration; the NGX SDK is not bundled and distributable builds disable it. [README]
- Multithreading: enkiTS worker jobs for selected terrain, cache, texture conversion and audio work, plus async streaming and pipeline builds. The README says this does not mean every system is threaded. [README] Partial.
- GitHub language bytes: C++ about 37 MB, Rust 6 MB, WGSL 2 MB, PowerShell 2.6 MB, SQF 235 KB.

### Rendering and lighting

- GTAO (screen-space AO with bent-normal ambient), live dev controls and debug views. [README]
- Global illumination from a local irradiance probe grid: sky bounce and coloured sunlight bounce. [README]
- Dynamic sun and local-light shadows, contact shadows, muzzle illumination, atmospheric light scattering, script-created lights (Arma 2/3 light commands). [README]
- Vehicle clear-coat paint reflecting sky and ground, with screen-space reflections; glass, chrome and instruments distinguished from paint. [README]
- Volumetric clouds and a procedural sky (stars, Milky Way, cloud shadows on terrain, objects, grass and water); overcast controls shared with Zeus. [README]
- Layered volumetric fog: adjustable heights, density and patchiness, valley and mountain presets, a legacy-fog option, local light scattering for night lights and the torch. Known limitation: "dense/aerial fog can still show patterns". [README]
- GPU-driven tree culling and drawing, including Arma 3 and DayZ vegetation. [README]
- Satellite terrain textures: authored satmaps plus an automatic far-colour bake for classic worlds. [README]
- Procedural grass with travelling gust fronts and interaction with players, vehicles, explosions and downwash. [README]

### Worlds and content

- Whole-world rendering: ground to orbit over the whole Earth from real elevation and imagery streamed at runtime (needs internet). Mountain shadows, sea to the horizon, valley fog, seasonal snowlines. A real-time Moon with relief and craters. A scripted ICBM can be flown across the globe. [README] Opt-in. Added in the 2026-10-25 commit; an "opt-in Earth terrain streaming preview" came 2026-10-07.
- Arma 1/2/3, DayZ and Reforger terrains load from the local install with buildings, vegetation, roads and materials; Reforger worlds stream natively; imported terrain opens in the native OFP mission editor with OFP units. [README, COMPAT] Partial: "material interpretation, object streaming and model/LOD compatibility are incomplete for some modern content", and an imported map "is not a port of that game's simulation, missions or configs".
- Arma 1/2/3 missions and campaigns (experimental) run from their native menus with their own weapons, sounds, voices, animals, state machines and conversations. Arma 1 is "furthest along". Some missions stop at missing script commands or content. [README]
- Island Editor ("no object limit") and "OFP Fusion", an archipelago built from the five locally installed classic islands (`content/fusion`). [README]
- Local content browser and editor: terrain discovery across installed games, partial format conversion, selected vehicle bridges. COMPAT: vehicle compatibility "does not promise that arbitrary modern vehicles work".

### Fire, smoke and destruction

- Volumetric fire and smoke: fireball explosions, wrecks burning with wind-leaning flames, buoyant smoke with wind response, self-shadowing and room/portal-aware containment with doorway flow (depends on building topology). [README]
- Volumetric impact craters with thrown-out earth. [README]
- Bullet and explosion holes in walls. [README]
- Destructible terrain and editor: brush deformation, small cuts, horizontal caves and tunnels, sinkholes, trenches, excavation editing. "Not a general voxel-world simulation." Object-cut terrain holes ported from Malprave. [README, CREDITS]
- Muzzle flashes that light the surroundings, ground explosions, snow bullet tracks, helicopter downwash particles. [README]

### Water, weather, wildlife

- Tidewater ocean (MIT port): FFT waves, shoreline wetness, reflections, foam, wakes, spray, caustics, underwater rendering, projectile/explosion/helicopter interaction. Boats are rigid bodies floating at real draft on the drawn waves. [README, CREDITS]
- Procedural wildlife and coast: fish schools, reef life, rippled sand, driftwood, surf, gulls, terns, whales. [README]
- Rain puddles that ripple and drain; wet ground and clothing. [README]
- Snow depth, compressed foot and vehicle tracks, projectile marks, roof and vegetation snow cover. [README]
- Mud ruts, rolling resistance, mud thrown from wheels and tracks. "Soft-ground prototypes do not provide general soft-body vehicle physics." [README] Limited.

### Soldiers, AI, physics, gameplay

- Box3D physics with articulated ragdolls for death and explosion reactions; on by default, toggle in the Physics dev tab. Tested on 37 stock models; two Camel pilot models still have settling issues. Posture, saved-corpse reactivation, network ownership and physics budgets limit ragdoll admission. [README]
- New infantry animations: reloading while walking, Q/E leaning, jumping with soft landing, swimming, diving and surfacing. [README]
- Swimming engine ported from Malprave: sea routes, boat boarding and exits, swimmer movement. Generated moves in `content/default-packs/swim`. [README, CREDITS, ASSETS]
- Improved AI (Malprave-derived): gunfire hearing and reporting, immediate orders, near-miss suppression, remembering where danger came from and searching there, turning on shooters, looking out from cover, avoiding cover where a squad mate was hit, running from live grenades; optional gradual spotting and cover/flank/search behaviour. "Not a universal new navigation system or complete replacement AI"; switches are config-dependent. [README, CREDITS]
- NPC chat (experimental): talk to squad, nearby soldiers, civilians and HQ by typing or voice; a language model answers in character from a live picture of the situation plus the mission briefing. Hosted providers or local models (Ollama, any OpenAI-compatible server). Replies spoken in 3D or over radio. Plain-language orders (follow, hold, move, cover, engage, get in) run through the game's own commands; "model output is never run as script"; API keys stay local. [README]
- Physical inventory (single-player): Tab on foot (O in vehicle control seats), vicinity cargo, drag and drop, equipped slots, weight and volume budgets; K opens inventory settings; 17 original equipment icons. Multiplayer keeps classic Gear. [README]
- Firing from vehicles: personal-rifle fire from configured passenger seats, aiming arcs, reload, Ready/Stow actions. Stock Jeep validated; other seats, weapon families and multiplayer limited. [README]

### Audio

- Steam Audio default backend (`engine/PoseidonSteamAudio`): HRTF binaural spatialisation, terrain and building occlusion, room reverb, underwater sound. [README]
- Classic OpenAL backend remains available. [README]
- Sea soundscape: surf wave by wave, gulls, terns, whales, boat engines. [README]

### Scripting, modding, config

- `getUnitLoadout` / `setUnitLoadout`, ported from jschmidt92/cwr_ce_source. CWR-CE loadout format, not Arma 3's; no clothing or inventory grids. Doc: `docs/scripting/unit_loadout_commands.md`. Merged 2026-10-15.
- In-memory key-value database: `dbNew`, `dbLoad`, `dbSave`, `dbSet`, `dbGet`, `dbExists`, `dbDelete`, `dbKeys`, `dbList*`, `dbHash*` and others; binary `PDB1` format under `%APPDATA%/CWR/Users/<profile>/Config/`. Doc: `docs/scripting/in_memory_database.md`. Multiplayer untested.
- Arma 2/3 light commands and the Arma 1/2/3 commands missions need, partially covered.
- `dev-missions/` (smoke-room, tidewater-boat, snow-road, jeep-ffv, swim-*, ugcave-test, terrain-brush, database-loadout and others) used as smoke and regression tests.

### Tools and diagnostics

- Inherited targets: `PoseidonGame`, `PoseidonGameDemo`, `PoseidonServer`, `PoseidonTools`, `PoseidonEvaluator`, `PoseidonStudio`, `PoseidonTetris`, `TcPbo`, `TcLister`; `engine/PoseidonFormats` C API backing a Blender importer; Rust `Trident` test runner and `mserver`. [APPS]
- Developer overlay: GPU timings, debug views, live render controls, Zeus free-fly camera with unit and vehicle placement, `--diag` event log with `diag_*` harness commands (from Malprave's mwDiag). Zeus and Dev Tools disabled in multiplayer. [README, CREDITS]
- Format support: Arma 1/2/3, DayZ and Reforger world and model readers; DDS masked/uncompressed layouts, DXT2/DXT4 and DXT1 punch-through (from Malprave). Conversion is "partial". [README, CREDITS]

### Multiplayer and networking

No specific new networking claims. Server and master-server code are inherited from CWR-CE. Multiplayer keeps classic Gear, Dev Tools and Zeus are disabled, FFV is limited, and ragdoll network ownership limits admission. Treat MP as unverified and not a focus.

## Planned or in progress

The README has no roadmap section. The only forward-looking text is "Current limitations": fog patterns, ragdoll edge cases, weather materials and modern-game material/LOD conversion "still need refinement", plus Arma 1/2/3 mission gaps.

The only roadmap-style checklist in this family belongs to the sibling `koosoli/PoseidonVK` and is not claimed by Open Poseidon: Vulkan raster parity (partly done), glTF 2.0 import, BC7 textures, FSR after an HDR chain, Gerstner water, Forward+ lighting, GPU culling and indirect draws, GPU particles, 6DoF, swimming, SDL3 gamepad mappings, multiscreen UI, OpenXR VR, async radio audio, animated loading screens.

## Reading notes

- Many features are claimed as shipped, mostly in a "development capture" sense with local content and no binary fixtures in the repo.
- The engine base is the CWR-CE lineage, not the Remaster tree directly. The README does not say when it last rebased on CWR-CE.
