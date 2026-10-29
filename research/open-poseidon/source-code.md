# Open Poseidon: source-code findings

Researched 2026-10-29 from shallow clones of koosoli/OpenPoseidon, ofpisnotdead-com/CWR-CE (tip `aa52258`, 2026-10-07) and paavohuhtala/CWR-CE. Nothing was built, run or tested.

## Identity from the code

- Confirmed as the OFP/Poseidon engine: `engine/Poseidon/AI/*`, `ParamFile`, PBO and `*.sqs` handling, `GPL-3.0-or-later` headers.
- Created 2026-10-05, last pushed 2026-10-25; about 140 MB; 16 stars, 6 forks, 0 open issues.
- 2 contributors: koosoli ("Deus-Ex", 14 commits) and jschmidt92 (13 commits); 27 commits on `main`.
- Not a GitHub fork (`fork:false`). The first commit `250374d4` (2026-10-07, "Open Poseidon Engine source release") shares no history with CWR-CE. The only visible history is the 26 commits after it, including `c57d59f0` (2026-10-25, "Arma 1/2/3 missions, whole-world rendering, fire and smoke, wildlife, NPC chat").
- `CREDITS.md` names paavohuhtala/CWR-CE as the base. The exact CWR-CE commit could not be confirmed.
- `LICENSE` is Bohemia's GPLv3-or-later plus Section 7 terms, verbatim. `THIRD_PARTY_NOTICES.md` and `ASSETS.md` document third-party content.

## Size

GitHub language bytes: C++ 37.3M, Rust 6.0M, PowerShell 2.6M, WGSL 2.1M, Python 1.0M, SQF 0.23M, CMake 0.19M.

| Area | Lines |
|---|---|
| `engine/Poseidon` | about 679k |
| `engine/WgpuRenderer` (C++ and Rust) | about 220k |
| `apps` | about 49k |
| `engine/PoseidonGL33` | about 10.5k |
| `engine/Trident` | about 8.6k |
| `engine/PoseidonSteamAudio` | about 6.1k |

4,778 tracked files, against 2,933 in CWR-CE.

## Layout and build

- Top level: `engine/`, `apps/`, `mserver/`, `tools/`, `content/`, `assets/`, `scripts/`, `docs/scripting/`, `tests/`, `cmake/`, `dev-missions/`.
- CMake 3.25+ with Ninja, clang-cl and vcpkg manifest mode (overlay ports in `cmake/vcpkg-overlay-ports`), plus a Cargo workspace: `engine/Trident`, `engine/WgpuRenderer/rust`, `mserver/{Archive,CLI,Client,MasterService}`. Rust edition 2024, MSRV 1.87.
- Python with Pillow and NumPy for content generation.
- Presets in `cmake/presets/*.json`: Windows `win-x64-clang-{dbg,rwdi,rel}`; Linux `linux-x64-clang*` and `steamrt4-x64-clang-*` (Steam Runtime 4); macOS arm64 and x64; sanitizer, TSAN and fuzz toolchains.
- **No CI.** `.github` holds only `FUNDING.yml`. CWR-CE's `build`, `ci`, `lint`, `pch` and `rust` workflows and its Linux arm64 toolchains were dropped.
- Tests: `tests/{unit,integration,perf,e2e,smoke,stress,manual}` with Catch2. Binary fixtures omitted for provenance, so the asset-dependent suite cannot pass from the export alone.

### Build and platform matrix

| Item | Value |
|---|---|
| Primary | Windows 10/11 x64, clang-cl, Ninja, vcpkg, Rust stable |
| Windows renderer | wgpu (Vulkan or D3D12) in a Rust DLL, GL33 fallback |
| Windows audio | Steam Audio default, OpenAL fallback |
| Linux | `linux-x64-clang*`, `steamrt4` presets; no runtime parity promised |
| macOS | arm64 and x64 presets; same caveat |
| Linux arm64 | present in CWR-CE, removed |
| CI | none |

Whether any non-Windows preset configures or builds was not checked.

## Features by subsystem

### Rendering

- New Rust/wgpu backend: `engine/WgpuRenderer/` (C++ glue) plus a Rust cdylib in `engine/WgpuRenderer/rust` (`wgpu_renderer.dll`; wgpu 30, naga_oil, ash). Enabled by `POSEIDON_ENABLE_WGPU` (default ON) in `CMakeLists.txt`.
- `wgpu::Backends::DX12` selection in `engine/WgpuRenderer/rust/src/lib.rs` plus `Backend::Vulkan` code paths.
- `engine/PoseidonGL33` remains but is thinner than CWR-CE's: `EngineGL33_ShaderSources.cpp` and `EngineGL33_Terrain.cpp` are missing.
- WGSL and Rust modules under `engine/WgpuRenderer/rust/src/`:
  - Lighting and shadows: `shaders/{lighting,shadow,gbuffer,shading}.wgsl`, GTAO, GI probes, SSR.
  - Post: `bloom`, `dof`, `exposure`, `godrays`, `tonemap`, `temporal`, `fsr1.wgsl`.
  - DLSS: `dlss*.rs` behind a `dlss` Cargo feature, off by default because the NGX SDK is proprietary and GPL-incompatible (comment in `Cargo.toml`).
  - Fog and sky: `layered_fog.*`, `sky/`.
  - Water: `water/`, `water_tw/`, `river/`, `underwater*.wgsl`, `rain_water*.rs`.
  - Surface and weather: `mud/`, `grass/`, `fire/`, `smoke_volume.*`, `shaders/snow_material.wgsl`, `ground_puddles.wgsl`, `terrain/`.
- C++ side: wildlife in `BirdFlocks`, `FishSchools`, `ReefLife`, `BeachCritters`; trees in `ProceduralTrees`; water in `WaterWgpu` and `TerrainWgpuTidewater*`; GPU-driven forests via `thirdparty/meshoptimizer-clod` and `CdlodDriver.hpp`.
- Whole-Earth streaming in `engine/WgpuRenderer/Earth*.cpp` and `engine/Poseidon/World/Terrain/Earth*.hpp` (`EarthMoonRelief`, `EarthFreeFlight`, `EarthGravity`). Hardcoded data sources: ESA WorldCover on AWS, NASA GIBS VIIRS Black Marble, EOX Sentinel-2 cloudless tiles, Overture Maps STAC/tiles. Moon data (NASA LRO/LOLA) and star catalogues in `assets/earth/moon` and `resources/sky`.
- Fire and smoke: `engine/Poseidon/World/Effects/{SmokeSystem,SmokeVolume,SmokeDensityGrid,FireVolumes,BuildingBreach,BuildingInterior,MudSpray}.*`. Craters in `World/Terrain/CraterField.hpp`. Cave editor in `Dev/Diag/CaveEditor.*`.

### Audio

- `engine/PoseidonSteamAudio/`: HRTF, `Occlusion.cpp`, `Reverb.cpp`, EFX presets. `vcpkg.json` lists `steam-audio` for Windows only.
- `engine/PoseidonOpenAL` retained.
- Ocean soundscape: `World/SeaSoundscape.cpp`, `World/EnvSoundRegions.*`. Opus, Vorbis, OpenAL-soft.

### Input

- SDL3 (`vcpkg.json` pins sdl3 >= 3.4.10), `Input/InputProcessingSdl.cpp`.
- Gamepad and controller UI (`Input/ControllerUi*.cpp`, `GamepadState.hpp`), `InputProfile`, Q/E lean via `InfantryController`. (UD's tree has the same controller UI and SDL input files, inherited from upstream.)

### Physics

- Box3D backend: `engine/Poseidon/World/Physics/Box3DBackend.cpp`, `PhysicsWorld.cpp`; ragdolls in `CorpseArticulation.hpp`; loose objects in `LooseObjects.*`. `box3d` is a vcpkg dependency.

### AI

- Files under `engine/Poseidon/AI/` that CWR-CE lacks: `AICombatPace`, `AIDangerList`, `AIGrenadeDanger`, `AIHunt`, `AIGroupScanBudget.hpp`, `AICostAccum`. These back the README's gunfire hearing, near-miss suppression, grenade avoidance and optional gradual spotting.

### Networking

- Mostly CWR-CE's: `Network/MasterServer*`, `MultiplayerAuth`, `IpBan`, `IntegrityCheck`, and `mserver/` (Rust Archive, CLI, Client, MasterService). No Open Poseidon-specific netcode was looked for.

### Scripting

- Command tables registered in `engine/Poseidon/Game/Commands/GameStateExt*.cpp` (about 35k lines).
- `GameFunction(..., "name")` registrations, same regex on each tree: CWR-CE 409, Open Poseidon 692, UD 531. Macro-registered commands are not matched, so these are lower bounds.
- New groups in Open Poseidon:
  - Arma 3-style language core: `params`, `param`, `compile`, `isEqualTo`, hash maps (`createHashMapFromArray`), `lnb*` and `ctrl*` UI commands, `getUnitLoadout`, `configClasses`, `execVM`, `execFSM` and more, in `GameStateExtA3.cpp`, `GameStateExtA3Functions.cpp`, `GameStateExtA3Rsc.cpp`, `GameStateExtA3Stubs.cpp` (UI, Eden and curator stubs). A file header says these were "written from the public description of each command... no Bohemia code is used".
  - Arma 2: `GameStateExtA2*.cpp`. Arma 1: `GameStateExtA1Config.cpp`.
  - Earth: `earthGeoToPos`, `earthPosToGeo`, `earthGoTo`, `earthGroundHeight`.
  - In-memory database (`db*`, by jschmidt92) and unit loadout commands, documented in `docs/scripting/`.
- Compat layers: `Game/Compat/{Arma1Compat,Arma3Compat,Arma3Eden,Arma2CampaignAuto,ArmaOriginalUi}`, `UI/Arma3Menu.cpp`, `UI/ArmaContentMenu.cpp`.
- `Game/Scripting/Coroutine.*` exists; whether it is new was not checked.

### File formats and asset loading

Under `engine/Poseidon/Asset/Formats/`:

- P3D: `P3D/{MLODLoader,MLODWriter,ODOLLoader,Odol40,Odol49,Odol73}`. Odol40 is OFP-era; Odol49 and Odol73 are later formats. MLOD writing too.
- World: `World/{Oprw20,Oprw24,Oprw25}`. Whether these read Arma 1/2/3 worlds was not confirmed.
- Material: `Material/{EmatMaterialAdapter,RvMaterialSource,ShaderSchema}`.
- RTM animation: `RTM/`.
- Enfusion/Reforger: `Enfusion/{PakArchive,EbinWorld,XobModel,XobCollision,PrefabResolve,TerrainMaterials}`, reading Reforger `.pak`, `.ebin` and `.xob` directly. Also `World/Terrain/LandLoadEnfusion.cpp`, `Enfusion{Doors,Glass}.cpp`.
- Local content browser: `UI/LocalMap*.cpp`, `UI/LocalVehiclesCatalog.hpp`.
- PAA/PAC, PBO and WRP handling retained from the base engine.

### Tools and apps

- `apps/tools/`: Studio, Island Editor (with DX12 and `shaders/`), TcLister and TcPbo (Total Commander plugins), Blender P3D import addon, Evaluator CLI.
- `tools/arma1-launcher`, `tools/reshade-bridge`, `tools/reshade-depth`, `tools/texture-workbench`, `apps/fuzzers`, `apps/tetris`.
- `content/fusion` (OFP Fusion archipelago), `dev-missions/`.
- `content/default-packs`: swim animations, a Tidewater boat and wildlife (MIT), ez-tree (MIT).

### LLM NPC chat

`engine/Poseidon/Dev/NpcChat/` with provider layers (`NpcChatSettings.hpp`):

- LLM: OpenAI, OpenRouter, Anthropic, Gemini, Ollama, custom OpenAI-compatible.
- Speech-to-text: OpenAI, Groq, Google, ElevenLabs, Gemini, custom.
- Text-to-speech: Windows SAPI, OpenAI, Google, ElevenLabs, Gemini, custom.
- Secrets handling (`NpcChatSecrets.cpp`), overlay UI, test harness.

### Other gameplay

Physical inventory UI under `engine/Poseidon/UI/Inventory/`; firing from vehicles; jump, swim and dive animations; destructible terrain; holes in walls.

## Divergence from CWR-CE

| Measure | CWR-CE (`aa52258`) | Open Poseidon |
|---|---|---|
| Tracked files | 2,933 | 4,778 |
| Files in common | 2,748 | 2,748 |
| Files only in this tree | 185 | 2,030 |
| Differing entries under `engine/Poseidon` (`diff -rq`) | | about 359 |

- The 2,030 added files: 537 under `engine/Poseidon`, 353 in `engine/WgpuRenderer`, about 270 unit tests, 104 default-pack files, 53 integration tests, 49 tools, 48 perf tests.
- The 185 CWR-CE-only files are mostly CI workflows, Linux docs, arm64 toolchains and GL33 shader and terrain files.
- Open Poseidon appears to add features without absorbing CWR-CE's later work: its GL33 backend is thinner and the CI is gone. Whether it tracks CWR-CE fixes after 2026-10-07 is unverified.

## Not verified

- Runtime behaviour of anything.
- Whether the shaders and effects work as described.
- mserver and netcode changes versus CWR-CE.
- The private OpenPoseidon-Builds repo.
- The CWR-CE base commit and the pre-export history.
- A full command-table diff (the regex undercounts macro-registered commands).
