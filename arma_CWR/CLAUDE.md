# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

This directory holds the engine and overhaul for **Uslu dur!** (**UD**) — a **total game overhaul** of *Arma: Cold War Assault* (originally *Operation Flashpoint*, 2001), not just an engine port. "Uslu dur!" is the public brand; the rename is branding-only, so the `cwr`/`CWR`/`Poseidon` codenames throughout the source are unchanged and stay as-is.

> **Repo layout note:** the git root is one level up at `D:\Arma_CWA`, not here. `arma_CWR/` is now a subdirectory alongside `site/` (the web portal) and `@LoBo/` (a mod test bed). See `D:\Arma_CWA\CLAUDE.md` for the repo-wide map; this file covers the engine + overhaul that live under `arma_CWR/`.

Two layers live here:

- **The upstream engine** (`engine/`, `apps/`, etc.), codename **Poseidon**. Built with CMake + Clang for Windows x64 and Linux x64 (macOS presets also exist). The root `CMakeLists.txt` defaults to C++20, but the core `Poseidon` lib and the `PoseidonGL33` backend override it to **C++17** with their `CXX_STANDARD` target property; apps, tools, tests, `PoseidonOpenAL` and `PoseidonFormats` build as C++20. Whether to raise the engine to C++20 is an open decision tracked on GitHub. This part is a **locked** Bohemia Interactive source release that accepts no upstream PRs. Our own CI lives at the git root: `.github/workflows/ci.yml` (issue #54 E3, first green 2026-09-03) builds the `linux-x64-clang-rwdi` preset and runs the unit suites and the mod-intake lanes (mod doctor, Guerrilla descriptor lint, roster probe) on synthetic fixtures, plus `tests/runtime/test-setup.ps1` on Ubuntu and Windows, all without game data; `lobo-repair.yml` runs `tests/ci/test_lobo_*.py` on both OSes when the LoBo repair tools change. Trident integration tests need game data and stay on developer machines. The code is GPL-3.0-or-later; game data (models, textures, sounds, missions) is **not** in this repo and ships separately under APL-SA.
- **The overhaul itself** ([`mod-plans/`](mod-plans/README.md), [`guerrilla-mode/`](guerrilla-mode/README.md)) — a from-the-ground-up rework of AI tactics/perception plus **Guerrilla Mode**, a new persistent open-world insurgency game mode with swappable islands and factions chosen at new-game. This is where active development happens. **Direction (2026-07-01): this is a total overhaul of the engine source, not a mod on a fixed binary — C++ engine changes are a first-class tool here, not a last resort, alongside SQF mission scripting.** Treat the mod-plans as living, AI-drafted design docs subject to correction, not settled hard specs.

## Target platform

UD targets the full official *Arma: Cold War Assault* Remaster; Classic 1.99 compatibility is being dropped (2026-10-11). Build and test new work against Remaster data and its `remaster.cpp` contract, and add no new Classic-only shims, fallbacks or test lanes; the Classic shims and data dir that remain are legacy awaiting retirement. The policy lives in the "Target platform" section of the repo-root [`CLAUDE.md`](../CLAUDE.md).

## Working map (where to look / what to skip)

`engine/` plus `apps/` is ~1,430 C++ files, ~160 of them over 1,000 lines. To avoid reading extraneous code:

**Active overhaul surface, start here:**
- `engine/Poseidon/Game/Guerrilla/` (~99 files): the native Guerrilla core, by group.
  - Campaign state: `ZoneRegistry` (zones, factions, capture), `GarrisonCache` (distance-cached garrisons), `AlertMachine` (per-zone GREEN/YELLOW/RED), `TownFlags`, `CounterAttack` (occupier town retakes) + `CounterAttack{Air,March,Orders}` (air packages, the march to the town edge, the vocabulary shared with the director), `CampaignEnd` (win rule and latch).
  - Occupier forces ([`guerrilla-mode/OCCUPIER-FORCES.md`](guerrilla-mode/OCCUPIER-FORCES.md)): `OccupierForces` (strategic pool and director) + `OccupierForcesLedger` (pool seams, ceiling, refill) + `OccupierForcesLevers` (the eight author levers), `ForcePackages` (what an operation buys), `OccupierAir` (air births, insertions, strikes), `DirectorLog` (per-action log level), `RoadRoute` (road-graph route search), `Base{Detect,Generate,Props}` (hub auto-detection, generated outposts and their layouts).
  - World population: `Traffic` + `TrafficEncounter` (road traffic, hold-ups, roadblocks), `Undercover` (per-observer side resolution), `AssailantSystem` (civilians who turn armed, #42).
  - Player base and economy: `GuerrillaBase` (HQ, weapon cache, garage), `StashRegistry` (keep-when-empty supply holders), `Market` (weapon and vehicle dealers).
  - Narrative: `Journal` (diary/objective data), `LegendRegistry` + `Legend{Names,Placement,Appearance,Seed}` (earned companion names, enemy Legend bosses), `FactionHistory` (seeded backstory), `WorldNames`, `Portrait{Service,Renderer,Recipe,Roster,Status}` (dossier portraits).
  - Content intake: `FactionSources` (the one faction table), `FactionTwins`, `OutfitSelect` + `AddonActivation` (launch-time seams), and the pure cores of the offline tools: `IslandScaffold`, `DescriptorLint`, `RosterProbe`.
  - SQF bindings: `<Name>Commands.cpp` exists for ZoneRegistry, GarrisonCache, CounterAttack, CampaignEnd, Traffic, Undercover, Assailant, GuerrillaBase, StashRegistry, Market, Journal, LegendRegistry, OccupierForces and OccupierAir; `AlertMachine` and `TownFlags` have none. `ScriptVars.hpp` reads script globals null-safely.
  - Every `.cpp` here (and every `guerrilla-mode/core/scripts/*.sqs`) must be listed in `guerrilla-mode/mission/Showcase.Abel/human/coverage.json`, or `tests/contracts/test_human_suite.py` goes red.
- `engine/Poseidon/UI/Guerrilla/`: `GuerrillaModule` (main-menu entry), `GuerrillaNewGame` (island/faction pick), `GuerrillaCharacterSelect` + `GuerrillaBodyPreview` (body browser, turntable), and the map-screen journal: `GuerrillaJournalPages` (gather), `JournalCompose*` (compose), `JournalRender`, `JournalText`, `JournalManual` (handbook chapters).
- Other engine touchpoints: `engine/Poseidon/Game/Commands/GameStateExt*.cpp` (the general SQF command table; `GameStateExtTest*` hold the `tri*` Trident commands), `engine/PoseidonGL33/EngineGL33_Portrait.cpp` (portrait capture), `apps/tools/Tools/commands/Guerrilla{,Lint,Probe,Scaffold}Command.*` (`PoseidonTools guerrilla lint|probe|scaffold`).
- `guerrilla-mode/` script and content side: `core/` (shared SQS policy layer, installed as `<GameDir>\gmcore\`), `mission/` (island templates, `Showcase.Abel`, single-feature test missions), `config/` (faction libraries, HQ classes).
- `engine/Poseidon/AI/`: the AI tactics/perception rework. See [`mod-plans/`](mod-plans/README.md) for which behavior maps to which files.
- `voice-lines/`: TTS for custom in-game speech. `generate_voice.py` writes WAVs via OpenRouter (`openai/gpt-audio-mini`); WAV or OGG both play through `CfgSounds` + `say`/`sideRadio`, no engine changes. Key comes from the gitignored repo-root `.env`.

**Design docs** (living AI drafts: treat them as intent, not a precise index, and verify against code): [`guerrilla-mode/README.md`](guerrilla-mode/README.md) (overview, links), [`ARCHITECTURE.md`](guerrilla-mode/ARCHITECTURE.md) (native core vs script layer; section A is current), [`STATUS.md`](guerrilla-mode/STATUS.md) (each requirement mapped to native or script), [`OFFSCREEN-CONTRACT.md`](guerrilla-mode/OFFSCREEN-CONTRACT.md) (what the campaign does where the player is not looking), [`OCCUPIER-FORCES.md`](guerrilla-mode/OCCUPIER-FORCES.md) (the occupier pool, director, packages and air), [`FACTION-PACKS.md`](guerrilla-mode/FACTION-PACKS.md) and [`ISLAND-PACKS.md`](guerrilla-mode/ISLAND-PACKS.md) (modding contracts), [`FULL-SCALE-SESSION.md`](guerrilla-mode/FULL-SCALE-SESSION.md) (Remaster shadow setup, long-session runbook), [`HUMAN-TESTS.md`](guerrilla-mode/HUMAN-TESTS.md) (43-case Showcase acceptance suite), [`mod-plans/`](mod-plans/README.md) (AI rework).

**Upstream code the overhaul does change:** `World/` (`WorldInit.cpp` mission init, `World.cpp` per-frame hooks, `WorldImpl.cpp` save/load of the Guerrilla services, `Detection/Target.cpp` undercover side resolution) and `UI/Map/` (map screen, journal notepad) take regular Guerrilla commits. **Frozen, read as reference:** `Network/` (locked Bohemia source release). `Graphics/` and `PoseidonGL33/` are under active rework on the graphics-remaster, lighting and water-research branches; on main they are reference only.

**Do NOT read or grep these, generated or non-source, pure context waste:**
- `thirdparty/glad/`: ~22k lines of generated OpenGL bindings.
- `build/`, and at the repo root `@LoBo/` (mod test data, 2.2G binary), `ARMA Cold War Assault [Classic]/` and `Arma Cold War Assault Demo [Remaster]/` (whole game-data folders). Not reading @LoBo is about context cost, not permission: **@LoBo is ours to modify as of 2026-08-08** (see the repo-root `CLAUDE.md`). Repair a content defect in its pbos rather than shimming around it, and land the repair as a rerunnable script under `tools/lobo/` because the directory itself is gitignored.
- The repo-root `D:\Arma_CWA\.ignore` excludes all of the above from `Grep`/`Glob`; the note here is for `Read`. Also gitignored data or output: `packages/` (junctions to game data), `dist/`, repo-root `tmp/` (Remaster shadows).

**Navigating big files efficiently:** grep for the symbol first, then `Read` a ±80-line window (`offset`/`limit`) rather than the whole file (the largest run 3,000 to 5,600 lines: `CounterAttack.cpp` is ~5,600, over the `FileSize` target's 5,000-line error limit (`cmake/CheckFileSize.cmake:21`); `Traffic.cpp` and `OccupierForces.cpp` ~4,700; `ZoneRegistry.cpp` ~4,300). For open-ended "where/how does X work" questions, delegate to an `Explore` subagent so the file dumps stay in *its* context and only the conclusion returns to yours.

## Toolchain prerequisites

Builds require, on PATH / in env:
- **Clang** — the only supported compiler (`clang-cl` frontend on Windows, GNU-driver clang on Linux and macOS). No MSVC, no GCC.
- **Ninja** — the generator used by every preset.
- **vcpkg** — manifest mode (`vcpkg.json`). `VCPKG_ROOT` env var must point at the vcpkg checkout: the base preset uses `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake` as its toolchain file, and each concrete preset chainloads a clang toolchain from `cmake/toolchains/` via `VCPKG_CHAINLOAD_TOOLCHAIN_FILE`.
- **ccache** — set as the compiler launcher by the base preset.

## Local environment (this Windows machine)

**Engine location:** `D:\Arma_CWA\arma_CWR` (moved here from `C:\dev\arma_CWR` on 2026-06-28 to sit next to the game data). The **git root is the parent** `D:\Arma_CWA` — run git and path-relative commands accordingly. A reminder shortcut `C:\dev\arma_CWR.lnk` points back here.

**Game data (not tracked source; treat read-only except where a task documents an exception):**
- **Full official Remaster (the target):** `D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered` (Steam app 65790). Never run setup against the Steam folder itself or add tests that read it directly (the `packages/Game` pins in Tests predate this rule). Setup, `run-game.ps1` and the `remaster_ud` test lane use a setup-prepared *shadow* of it, `D:\Arma_CWA\tmp\remaster-shadow-67` (built by `tools/runtime/new-remaster-shadow.ps1`, then `setup-guerrilla.ps1` in Remaster mode; see [`guerrilla-mode/FRESH-INSTALL.md`](guerrilla-mode/FRESH-INSTALL.md)).
- `D:\Arma_CWA\ARMA Cold War Assault [Classic]`: the v1.99 install (AddOns, Campaigns, Worlds, `ColdWarAssault.exe`). **Legacy, awaiting retirement.** The local `.trident.env` may still point `OFPR_DATA_DIR` here. It has no `@LoBo` inside it, so it cannot run the `lobo` lane (see Tests); point `OFPR_DATA_DIR` at a shadow instead.
- `D:\Arma_CWA\Arma Cold War Assault Demo [Remaster]`: the official remaster Demo (Steam app 4819000), read-only reference. **Its asset set is not a gameplay target** (asset-targeting policy, see the Demo bullet in the repo-root `CLAUDE.md`): Guerrilla content targets the full Remaster + `@LoBo` assets, and the plan-15 descriptor-resolution pass keeps missing classes non-fatal (logged substitution, never fatal or sterile). Surviving roles: (1) contract source: its `BIN\remaster.cpp` inventories the `Remaster >>` config keys the engine reads (#11); (2) reference binary `PoseidonGameDemo.exe` for upstream-behaviour diffs (#13); (3) occasional `demo`-lane data for engine-level (non-content) tests. The three Demo-world Guerrilla tests stay unrunnable locally, and making them run on Demo data is **explicitly not a goal** (see Tests below).

Both bracket dirs are wildcard-hostile: quote the path and use `-LiteralPath` in PowerShell (#12). Per-run `--data-dir` usage lives in the Tests section.

The toolchain is installed and verified as of 2026-06-28. Versions and locations on this machine:

- **Clang/LLVM 22.1.8** — installed via `winget` (`LLVM.LLVM`) to `C:\Program Files\LLVM\bin`. The LLVM winget package does **not** add itself to PATH; `C:\Program Files\LLVM\bin` was added to the **User** PATH manually.
- **CMake 4.3.3** — `winget` (`Kitware.CMake`), `C:\Program Files\CMake\bin` (on PATH).
- **Ninja 1.13.2** — `winget` (`Ninja-build.Ninja`), in the WinGet Packages dir (on PATH).
- **ccache 4.13.6** — `winget` (`Ccache.Ccache`), in the WinGet Packages dir (on PATH).
- **vcpkg** (tool 2026-05-27) — **git-cloned** to `C:\dev\vcpkg` and bootstrapped (not the packaged version). `VCPKG_ROOT=C:\dev\vcpkg` is set in **User** env vars. vcpkg is intentionally **not** on PATH — the build finds it through `VCPKG_ROOT`.
- **git** — pre-existing at `C:\Program Files\Git`.

Gotchas: PATH/env changes only apply to **newly opened** terminals. The first `cmake --preset` is slow (vcpkg compiles all deps from source once, then caches). Target platform/build for this machine: `win-x64-clang-rwdi`.

## Build

Configure with a preset, then build its binary dir (presets are *configure* presets only — there are no build presets):

```sh
cmake --preset win-x64-clang-rwdi          # Linux: linux-x64-clang-rwdi
cmake --build build/win-x64-clang-rwdi
```

Presets live in `cmake/presets/{windows,linux,macos,sanitizers}.json` (each includes `base.json`), pulled in by `CMakePresets.json`. Suffix = build type: `-dbg` Debug, `-rwdi` RelWithDebInfo, `-rel` Release.
- **Windows:** `win-x64-clang-dbg` / `-rwdi` / `-rel`, `win-x64-clang-san`, `win-x64-clang-fuzz`.
- **Linux:** `linux-x64-clang` (the Debug preset, no `-dbg` suffix), `linux-x64-clang-rwdi` / `-rel` / `-san` / `-tsan` / `-fuzz`. Steam Runtime: `linux-x64-steamrt4` (RelWithDebInfo) and `steamrt4-x64-clang-dbg` / `-rwdi` / `-rel`.
- **macOS:** `macos-arm64-clang` and `macos-x64-clang` (Debug), each with `-rwdi` / `-rel` / `-san` / `-tsan` variants.

The two `-fuzz` presets turn on `POSEIDON_BUILD_FUZZERS`. Each preset's binary dir is `build/<preset>`. Compiled apps are staged into a `dist/` dir named from the build dir's basename (`DIST_PRESET_NAME` in `CMakeLists.txt`): `<platform>-<arch>-clang-<suffix>` becomes `<arch>-<platform>-<suffix>` (`build/win-x64-clang-rwdi` -> `dist/x64-win-rwdi`, `build/linux-x64-clang` -> `dist/x64-linux`), `steamrt4-x64-clang-<suffix>` becomes `x64-steamrt4-<suffix>`, and any other basename is used unchanged. On this machine `build/` also holds non-preset dirs (`agent-rwdi`, `agent2-rwdi`, `agent67-rwdi`, `agent71-rwdi`, `lean-rwdi`, `options-rwdi`, `ud66-rwdi`, `ud66fix-rwdi`), all configured from `win-x64-clang-rwdi` so parallel agent sessions don't share a build or dist dir: `cmake --preset win-x64-clang-rwdi -B build/agent-rwdi` builds into `build/agent-rwdi` and stages to `dist/agent-rwdi`.

**clang-cl/Ninja dependency tracking (2026-10-05):** `cmake/ClangClDependencyTracking.cmake` uses `/showIncludes` for C and C++. With CMake 4.3's default forwarded GCC depfile flags, ccache 4.13.6 cache hits restore the object but lose its header dependencies. This left August renderer objects linked against September engine interfaces and broke world, journal and portrait rendering. Reconfiguring and rebuilding with the corrected flags recompiles existing objects and repairs the dependency database; keep the module enabled even when an incremental build reports success.

Run `python tests/ci/test_clang_cl_dependencies.py` with the toolchain on PATH to check real cache hits and header-change invalidation for C, C++ and PCH builds. The test uses its own compile-only fixture and cache under `tmp/`; explicit tool paths are also supported (`--help`).

## Run

For a fresh installation (the official Remaster, or legacy Classic) + LoBo, use
`setup-guerrilla.ps1` followed by
`play-guerrilla.ps1`, with explicit data/mod/binary paths. See
[`guerrilla-mode/FRESH-INSTALL.md`](guerrilla-mode/FRESH-INSTALL.md). The runtime
shims and licensed UI/font payload now have tracked sources; do not restore the
old local Microsoft-font workaround. The older launchers below remain available.

All four scripts (`run-game.ps1`, `run-game-with-mods.ps1`, `setup-guerrilla.ps1`,
`play-guerrilla.ps1`) live in the engine root `D:\Arma_CWA\arma_CWR`, not the git root.
`run-game.ps1` is the fast path to eyeball a change: it handles the
toolchain-on-PATH gotcha, does an incremental `PoseidonGame`-target build, and launches
the exe with `--window --no-splash` against the game data dir. Other params: `-Preset`
(build dir, default `win-x64-clang-rwdi`), `-LogDir` (default `logs\`) and `-ExtraArgs`
(raw args appended verbatim).

```powershell
.\run-game.ps1                                      # build + launch to the main menu
.\run-game.ps1 -Mission 'Missions\Guerrilla.Abel'   # build + jump straight into a mission
.\run-game.ps1 -SkipBuild                           # launch the existing dist/ build as-is
```

Default `-DataDir` is the setup-prepared Remaster shadow (`Get-UDDefaultDataDir` in
`tools/runtime/UDDataDir.psm1`). `$env:UD_DATA_DIR` wins if set, and throws if it is not
a directory (no fall-through). Otherwise the first of: the target of `packages\RemasterUD`,
`D:\Arma_CWA\tmp\remaster-shadow-67`, `D:\Arma_CWA\tmp\remaster-shadow`, each counting
only if it holds a `dtaExt` folder. If none qualifies it falls back, with a
warning, to the legacy `D:\Arma_CWA\ARMA Cold War Assault [Classic]`. `guerrilla-mode/install-missions.ps1`
and `run-game-with-mods.ps1` share that default. The shadow has no `demo` world, so
`Guerrilla.Demo` is not installed there. `-Mod`/`-Mods` mounts mod folders (one
`--mod` value, `;`-joined, mount order kept). `-Mission` is resolved relative
to `-DataDir` and passed as `--test-mission` (omit it to boot to the main menu, where
GUERRILLA lets you pick island/faction interactively). `run-game-with-mods.ps1` wraps
`run-game.ps1`: `-Mod` defaults to `@LoBo`, `@lobofixup` is appended after it unless
`-NoLoBoFixup`, and `-ModsDir` (default `D:\Arma_CWA`) is passed as `--mods-dir`.
`play-guerrilla.ps1` needs PowerShell 7 (`#requires -Version 7.0`) and mandatory
`-GameDir`/`-GameExe`; it captures the same three log files below by default and takes
`-Dev` (adds `--dev`) and `-NoLog`.

### Console-output capture (default on — read the files, don't run the exe in a terminal)

`run-game.ps1` launches the game **detached** and captures all console output into
timestamped files under `logs\` (gitignored), so a play session never dumps output into
the terminal/AI-session transcript. Per run it writes:

- `logs\game-<ts>.log` — the engine log (`--log-file` spdlog file sink; everything the
  engine logger prints to the console also lands here)
- `logs\game-<ts>.stdout.log` — raw stdout
- `logs\game-<ts>.stderr.log` — raw stderr; **crash-handler stack traces, minidump paths
  and CLI errors write directly to stderr**, bypassing the engine logger — this file is
  where to look after a crash

To follow a live session, `Get-Content <log> -Wait -Tail 50` in a background shell, or
just Read/Grep the newest `logs\game-*` files after the run. Diagnostics knobs when you
need more detail: `--log-level`, `--log-categories`, `--log-format` and `--logfiles`
(file-operation logging) — see `PoseidonGame.exe --help` / `AppConfig.cpp`. Pass
`-NoLog` to `run-game.ps1` to disable capture; never launch `PoseidonGame.exe` in the
foreground of an AI session to "see" its output — use the log files. `-NoLog` passes no
`--no-log-file`, so a detached run with no console still gets the engine's own auto log
at `<UserContentDir>\logs\cwr_*.log` (`apps/cwr/GameBase/GameBase.cpp`, `WantsRunLogFile`
plus `ShouldWriteAutoLog`).

**Always close what you open.** No human is at the keyboard to close game windows, and a
lingering instance poisons the next Trident connection ("connection error: eval failed").
Every automated command that spawns game instances (run-game.ps1, `tri test`) must
guarantee cleanup of **its own** instances in the same command. **Never kill
`PoseidonGame` globally** (`Get-Process PoseidonGame | Stop-Process` and the like are
forbidden): other sessions run Trident tests on this machine at the same time, and a
global kill ends their runs with exit -1 and an empty stderr. Use the scoped
`tools/stop-owned-games.ps1` instead:
- `tri test` games normally need nothing: Trident kills its own child (`kill_on_drop`,
  `engine/Trident/src/client/instance.rs`). After a crashed tri run, stop that run's
  games by data dir: `powershell -NoProfile -File tools/stop-owned-games.ps1 -DataDir <your shadow>`
  (matches the `-C <dir>` tri puts on the game's command line).
- run-game.ps1 prints the PID; stop it with `... stop-owned-games.ps1 -Pid <pid>` once done
  reading logs.
- By process tree: `-OwnerPid <pid>` (from Git Bash: `-OwnerPid $(cat /proc/$$/winpid)`).
  `-Orphans` selects games whose parent has exited; another session's run-game launch
  also counts, so prefer `-Pid`/`-DataDir`.

Give each agent its own shadow data dir under `D:\Arma_CWA\tmp\` and its own build dir
so `-DataDir` selection stays unambiguous (see `agent_guidelines/scoped-game-cleanup.md`).
Run `tri` with `--retries` (guerrilla_native_spawn's doMove assert is known-flaky) and,
before diagnosing connection failures, check for stray games with
`powershell -NoProfile -File tools/stop-owned-games.ps1 -DryRun` (lists every
`PoseidonGame` with PID, parent, age and data dir, and kills nothing).

## Tests

CTest runs the unit suites. Trident integration tests run through `tri test` (below), not
CTest: `cmake/TridentCTest.cmake` registers them only with `OFPR_REGISTER_TRIDENT_CTESTS=ON`
(default OFF, and OFF in `build/win-x64-clang-rwdi`), and even then only if a
`packages/Remaster` data dir exists, which nothing creates (issue #91).

```sh
ctest --test-dir build/win-x64-clang-rwdi --output-on-failure
ctest --test-dir build/win-x64-clang-rwdi -R "<test name>" --output-on-failure   # single test / regex
```

- **Unit** (`tests/unit/`, Catch2): suites `PoseidonFoundationTests`, `PoseidonCoreTests`, `PoseidonTests`, `PoseidonServerTests`, `PoseidonEvaluatorTests`, `PoseidonTetrisTests`. Catch2 tag filters like `[config]`, `[graphics]` partition `PoseidonTests`. Plus `PoseidonStudioTests` (`tests/unit/apps/Studio`), a Dear ImGui Test Engine suite for the Studio asset editor, registered via `imgui_discover_tests`.
- **Integration** (`tests/integration/`, Trident): SQF-driven in-game scenarios. Needs game data. Build the runner first, then run:
  ```sh
  cargo build --manifest-path engine/Trident/Cargo.toml
  tri test -j4 tests/integration               # --retries defaults to 3; binary: target/debug/tri[.exe] (Cargo workspace root is arma_CWR/; engine/Trident/target/ is a stale pre-workspace leftover)
  tri slots                                    # machine-wide game-slot limit, held slots, and the ledger of who runs what
  ```
  **Machine-wide game budget:** besides the per-run `-j`, every game-spawning `tri` command (`test`, `exec`, `stress`, `console`) takes `--global-slots <N>` (env `TRI_GLOBAL_SLOTS`; flag > env > default **4**; `0` = unlimited), a cap on simultaneous game instances shared by every `tri` on the machine. A test takes as many global slots as local ones (clamped to N). Slots are OS file locks in `%LOCALAPPDATA%\Trident\game-slots` (override `TRI_GLOBAL_SLOT_DIR`), released automatically if a `tri` dies; holders are listed in `D:\Arma_CWA\tmp\engine-slots\<pid>-<seq>.json` (override `TRI_SLOT_LEDGER_DIR`). A launch that waits logs `waiting for N global game slot(s)` with the holders, so a quiet run is queueing, not hung. Each `tri` enforces its own N; keep sessions on the default.
  Test layouts: `<name>.test.sqf` with an optional `<name>.test.toml` sidecar (tags, `data_dir`, `mission`, `extra_args`, timeouts); multi-instance `<name>.test/` directories whose `test.toml` lists `[[instances]]`; and sequences, a `<name>.seq/` directory of ordered `*.test.sqf` phases (each a fresh launch sharing one user dir; phases may carry their own `.test.toml`) with an optional sibling `<name>.seq.toml` for the sequence's tags.

  Copy `.trident.env.example` to `.trident.env` and set `OFPR_GAME_DIR` (build/dist dir with binaries) and `OFPR_DATA_DIR` (data for tests that pin no `data_dir`; a pinned `data_dir` wins over `--data-dir`). New setups target the full Remaster through a setup-prepared shadow, never the Steam folder itself: see "Development tests" in [`guerrilla-mode/FRESH-INSTALL.md`](guerrilla-mode/FRESH-INSTALL.md). The example file still says `packages/Demo`, pending the migration of the `full_cwa`/`lobo` lanes to Remaster data.

  **Unpinned tests**, the `full_cwa`/`lobo` lanes among them, run on whatever `OFPR_DATA_DIR` in `.trident.env` points at (or `--data-dir`). Since `e6bff81` the mod-lane tomls mount LoBo with `--mods-dir "." --mod "@LoBo"`, so the `lobo` lane (and the other unpinned tests that mount `@LoBo`, some `full_cwa` ones too) needs `<data dir>\@LoBo` inside the data dir: the Remaster shadow has it, the legacy Classic dir does not. A local `.trident.env` that still points at `ARMA Cold War Assault [Classic]` (as this machine's did on 2026-10-18) cannot run the `lobo` lane; point it, or `--data-dir`, at a shadow (see `guerrilla-mode/FRESH-INSTALL.md`, "Development tests"). Pinned tests use `data_dir = "packages/<name>"`, resolved against the repo root; each `packages/` entry is a gitignored **junction**, and without it a pinned test reports `spawn failed`, which is not a regression. Tests that reach repo fixtures through `--mods-dir ../../tests/fixtures/...` (relative to the game CWD) only land inside the repo when the data dir is one of these junctions; otherwise they die before `HARNESS_PORT`. Newer tomls avoid that: tri expands a `${repo_root}` token in `extra_args` to the engine checkout root (`arma_CWR`; `engine/Trident/src/scenarios/integration.rs:19`), so `${repo_root}/tests/fixtures/...` works from any data dir (about two dozen tomls use it). Remove a junction with `cmd /c rmdir <link>` (link only). Pins (counts drift; grep `data_dir` for current numbers):
  - **`packages/Demo`** (~75 tomls: the multi-instance `multiplayer` tests, the `ui` stock-menu/editor/options tests, and a few `harness`/`rendering` ones) -> the Demo package, created 2026-09-02 for issue #56 task 3: `cmd /c mklink /J "D:\Arma_CWA\arma_CWR\packages\Demo" "D:\Arma_CWA\Arma Cold War Assault Demo [Remaster]"`. An unpinned test can be pointed at it per run with `tri test --data-dir "D:\Arma_CWA\Arma Cold War Assault Demo [Remaster]" ...` (no globbing on the value, so the brackets are safe).
  - **`packages/Game`** (about a dozen tomls, mostly tag `full_game`) -> the unmodified retail Remaster (`D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered`). The mods-screen, join-requirements and a few menu/editor tests pin here because only that package ships the complete `BIN\STRINGTABLE_MAINMENU.utf8.csv` (`STR_DISP_MODS_LOAD` and the tab labels; the Demo carries a 231-key subset). The 32 menu/editor/mods tests that #49 part 1 listed as asserting on removed entries were re-pointed on 2026-09-02, not loosened: 21 to `packages/Demo`, 11 to `packages/Game`.
  - **`packages/RemasterUD`** (~90 tomls, tag `remaster_ud`) -> the setup-prepared Remaster shadow (`D:\Arma_CWA\tmp\remaster-shadow-67`; recipe in "Development tests", `guerrilla-mode/FRESH-INSTALL.md`). Covers `tests/integration/ui/remaster` (issue #67: the UD menu, Bohemia's Options/mods screens and Guerrilla boots, with and without @LoBo), the Guerrilla off-screen and counter-attack scripting tests, and the `occupierforces` suite (most of the pins on this branch). Never run setup against the Steam folder itself, and add no new tests that read it directly (the `packages/Game` pins predate the shadow).

  **The three Demo-world Guerrilla tests don't run on any local package, and per the 2026-07-14 asset-targeting policy this is accepted, not a work item.** `guerrilla_capture_flip`, `guerrilla_spawn_domove` and `guerrilla_save_reload.seq` boot `missions/guerrilla_capture.Demo` / `guerrilla_persist.Demo`, bound to the `demo` world, which neither Classic nor the full Remaster ships; and their `mission.sqm` player is `SoldierGB`, which the Demo [Remaster] package does not ship (no `SoldierG*` roster except `SoldierGFakeE`, #13), so a `--data-dir` run against the Demo package boot-fails within ~60 ms on the strict-mode config error. Descriptor-sourced spawns are already package-proof (the plan-15 resolution pass), but the `mission.sqm` class is not descriptor-sourced, and remapping it for Demo data is out of scope: the Demo asset set is not targeted. The runnable Guerrilla set here is the `remaster_ud` lane on the Remaster shadow, plus the `full_cwa` and `lobo` tests, which run on whatever data dir `.trident.env` names (a shadow, for the `lobo` lane; see above).

### Guerrilla integration-test preconditions (verified 2026-07-07)

- **Every Guerrilla mission now needs the shared script core installed** (issue #54 step B1, 2026-09-02): the managers live once at `guerrilla-mode/core` and are installed to `<GameDir>\gmcore` by `guerrilla-mode/install-missions.ps1`; the `Guerrilla.*` and `Showcase.Abel` templates carry a two-line `[] exec "\gmcore\init.sqs"` bootstrap, whose leading backslash resolves against the game data root rather than the mission folder. The reference slices `Qrf.Abel`, `Market.Abel` and `Undercover.Abel` carry their own longer `init.sqs` that execs `\gmcore\scripts\lib.sqs` plus the one core policy script it demonstrates (`qrf.sqs`, `market.sqs`, `undercover.sqs`), so they too need `gmcore`. This holds even for the tests that boot a repo-root-relative template path: the mission folder no longer carries `scripts/`, so without a `<GameDir>\gmcore` the mission boots with zero managers and the asserts go red on missing `gm*` globals. Re-run the installer after editing the core.
- `ui/guerrilla_new_game_e2e` needs the mission templates **installed in the game dir**: plain `guerrilla-mode/install-missions.ps1` (issue #54 C4, 2026-09-02: the world gate now accepts a loose `<GameDir>\Worlds\<World>.wrp` or a `<World>.wrp` entry in any pbo under `<GameDir>\AddOns\`, `<GameDir>\Dta\` or the `addons\` subfolder (not the root) of each `@*` mod beside or inside the game dir, or of each `-ModDir`, so Sinai and Lebanon80 install with no `-IncludeWorld`, which survives only as a manual override). The same run installs the shared script core (`gmcore`) and the **vanilla faction library** (`bin\guerrilla-factions.hpp` included from `bin\config-extra.cpp`, issue #54 A4). The @LoBo war rosters live once in `guerrilla-mode/config/lobo-factions.hpp`; **`tools/lobo/install-lobo-factions.ps1` writes them into `@LoBo\bin\config.cpp`** (issue #56 task 2, one-time, rerun after an @LoBo reinstall; `setup-guerrilla.ps1` already runs it on its `-LoBoDir`), and the `@lobofixup` test fixture `#include`s the same header. A session that mounts `@LoBo` without that install and without `@lobofixup` offers no war factions on Sinai/Lebanon80. `guerrilla_sinai_swap` and `guerrilla_lebanon80_boot` now mount `@LoBo` ALONE, so they need the roster install; they boot the repo-root-relative template path, so they need only the core + library install, not the templates.
- The LoBo tests mount `<data dir>\@LoBo` (a real copy inside the shadow, made by `new-remaster-shadow.ps1 -LoBoSource`), and that copy needs a one-time repair pass, idempotent and rerun after any @LoBo reinstall. `setup-guerrilla.ps1` already does it: it runs `tools/lobo/install-lobo-factions.ps1` and then `mod doctor --fix` on its `-LoBoDir`. To repair by hand, run it on that same copy, not on `D:\Arma_CWA\@LoBo` (unless the data dir is `D:\Arma_CWA` itself): **`dist/<build>/PoseidonTools.exe mod doctor "<data dir>\@LoBo" --fix`** (drop `--fix` to report only; `--pbo <wildcard>` narrows the model scan). It repairs all three defect classes in one go with same-length in-place byte patches (the pbo header table never moves) and keeps the originals in `@LoBo/_ud-orig`:
  - **UNDEFINED_SCOPE_KEYWORD** — `LoBoWreck.pbo` and `LoBoPalObj.pbo` are the only two @LoBo addons that omit the `#define private 0 / protected 1 / public 2` header every other @LoBo config carries, so their `scope = public;` resolved to nothing and read back as 0. Eighteen finished wreck and poster classes were refused with `Cannot create '<class>': type is abstract`. `scripting/prop_spawn_safety` CASE 2b goes red if the repair has not been run.
  - **BURIED_MODEL_ORIGIN** — `LoBo_M60A1_wreck` and `LoBo_M60A1_wreck2` were authored with their model origin **above** the mesh, and a static prop is seated at `terrainY + shape->BoundingCenter().Y` (`Entity::PlaceOnSurface`, Static branch, `World/Simulation/Simul.cpp:1300`), so `createVehicle [x, y, 0]` buried both tanks whole (roof 0.45 m / 0.92 m under the sand). Not a missing LOD: no LoBoWreck model has a LandContact LOD, including the eight that seat correctly. Since 2026-09-05 (ported from the `screenshots` branch) the engine also nets this case: `Entity::PlaceOnSurface` seats any model whose whole mesh sits at or below its own origin on its lowest vertex and logs a WARN naming the shape (`StaticSeatOffsetY`, `Vehicle.hpp`, pinned in `test_house.cpp`), so an unrepaired install still shows the tanks. Both layers resolve to the same seat; `scripting/prop_spawn_safety` CASE 2d asserts it and goes red only if the p3d repair is missing AND the engine net regressed. The same tranche made a config value that evaluates to nil (an undefined identifier such as `scope = public` without its `#define`) log a WARN once per spelling at the `ParamFileEval` seam instead of reading 0 silently.
  - **MALFORMED_FLOAT** — stock `@LoBo` ammo pbos carry `0.0.1` in eleven `tracerColor[]` arrays. The config reader now coerces such a token to its strtod prefix (see `ClassifyToleratedLiteral`), so this no longer hard-aborts under `--autotest`; the repair still normalises the bytes. On this machine `LoBoammo.pbo`/`LoBo_airammo.pbo` were already repaired in place on 2026-07-16 and are byte-identical to the `@lobofixup` fixture's shadows, so the shadows are inert; the fixture's `bin/config.cpp` (its `CfgAddons` preload roster and `CfgGuerrillaFactions`) is what still carries the lane.
  - `tools/lobo/fix-lobo-scope.ps1` and `tools/lobo/fix-lobo-model-origin.ps1` are now thin wrappers around `mod doctor`. `tests/fixtures/mods-lobo/@lobofixup/gen-patched-pbos.ps1` does more: it copies `LoBoammo.pbo` and `LoBo_airammo.pbo` from `@LoBo\addons` into the fixture, runs `mod doctor --fix` on the fixture, then deletes the fixture's `_ud-orig`. The logic lives in `engine/Poseidon/Asset/Addon/ModDoctor.{hpp,cpp}` and is unit-tested (`[moddoctor]`).
  - Where a reinstall comes from: **`D:\LoBo-Reloaded`** (outside the repo; family use only, never upload) is the LoBo source of truth on this machine. Built, verified packages sit in its `dist\` as `LoBo-Reloaded-<date>.7z`, unpacked under `modfolders\<date>\` as `@LoBo`, `@LoBo_Deps`, `@LoBo_UD` and `@LoBo_Extras`; they are built by `tools/lobo/package-lobo-family.ps1`, with the repair history in `tools/lobo/CONTENT-REPAIRS.md`. Per its `README.txt` (read it first), its `@LoBo` already ships with the mod doctor repairs applied.
- Run `tri` with `--retries` and low `-j`: `guerrilla_native_spawn` is known-flaky on its doMove displacement assert (recovers on retry), and parallel instances plus any manually closed window show up as `connection error: eval failed` cascades.

### Unit-test gotchas on Windows (verified 2026-06-28)

A clean build passes ~3536/3561 unit tests. The ~25 "failures" out of the box on this machine are **environment portability bugs in the tests, not engine regressions** — they reproduce on any fresh Windows/non-UTC checkout. Two causes, both fixed locally:

- **Hardcoded `/tmp/` paths (24 tests on 2026-06-28).** `test_paa_encoder` (`WritePAA`), `test_asset_preview` (`saveToFile` when `TMPDIR` is unset) and `test_network_server` (JIP world-state `.jips` writes) write to literal `"/tmp/..."`, and `test_gamePaths` points its env overrides there. A native Windows exe resolves that to `\tmp\` **on the current drive**, so those writes fail with no such directory. Fix: create `D:\tmp` (ctest runs from the `D:` drive). If you ever run the tests from another drive, create `\tmp` on that drive too. The other `"/tmp/` hits (grep of 2026-10-13) are harmless: `test_findfirst`, `test_dir_scanner`, `test_mission_info`, `test_game_state_ext` and `test_network_predicate_parity` use them only as string inputs, and `test_crash_handler`'s is in its POSIX-only branch. `test_dds_converter` no longer uses `/tmp`: it builds an exe-relative `tmp/` path and calls `create_directories`, the pattern to copy. Issue #90 tracks removing the hardcoded paths.
- **`FormatTime` timezone assumption (1 test).** `AssetInfo::FormatTime` correctly renders asset mod-times in **local** time; the test in `test_asset_info.cpp` assumed UTC and asserted the string contained `"2020"` for `1577836800`, which renders as `2019-12-31 16:00:00` west of UTC. Fixed the test to assert the `YYYY-MM-DD HH:MM:SS` *format* instead (production code unchanged — local-time display is the desired behavior).

## Lint / format

These are custom CMake targets (not standalone scripts) — build them against a configured dir:

```sh
cmake --build build/win-x64-clang-rwdi --target Format      # clang-format check (--Werror)
cmake --build build/win-x64-clang-rwdi --target FormatFix   # clang-format in place
cmake --build build/win-x64-clang-rwdi --target Tidy        # clang-tidy (Tidy/TidyFix exist only if clang-tidy is found)
cmake --build build/win-x64-clang-rwdi --target TidyFix     # clang-tidy --fix-errors --fix
cmake --build build/win-x64-clang-rwdi --target Lint        # Format, + ruff on the Blender addon if uv is found AND apps/tools/BlenderAddon/pyproject.toml exists (LintFix likewise)
cmake --build build/win-x64-clang-rwdi --target FileSize    # warn >3000 lines, error >5000
```

Rust crates (`engine/Trident`, `mserver/*`) are members of one Cargo workspace rooted at `arma_CWR/Cargo.toml` (shared `target/`, profiles `dbg`/`rwdi`/`rel`) and use the normal `cargo fmt` / `clippy` / `test` / `build`.

## Architecture

**Engine libs (`engine/`)**, the stack apps link against. `Poseidon` and `PoseidonGL33` build as C++17, the others as C++20 (see "What this is"):
- **Poseidon** (`engine/Poseidon/`, → `Poseidon.lib`): the whole engine in one static lib. Subsystems: `AI`, `Asset`, `Audio`, `Core`, `Dev`, `Foundation`, `Game`, `Graphics` (backend interface + factory), `IO` (streams, ParamFile, preproc), `Input`, `Network`, `Security`, `UI`, `World`. It also compiles the SQF evaluator from `engine/Evaluator/` and the RNG from `engine/Random/`; both sit at the top of `engine/`, not under `engine/Poseidon/`.
- **PoseidonGL33**: OpenGL 3.3 backend, linked only by the GUI clients (`PoseidonGame`, `PoseidonGameDemo`, `PoseidonTetris`). **PoseidonOpenAL**: OpenAL backend, linked by those clients and also by `PoseidonStudio` and `PoseidonTools`.
- **PoseidonFormats**: C API for P3D/PAA/PBO/RTM file formats, built from one source twice: a static `PoseidonFormats` lib (today linked only by `PoseidonTests`) and, on x64, a shared `PoseidonFormatsDLL` (output name `PoseidonFormats`) that statically links the whole `Poseidon` lib. The Blender addon at `apps/tools/BlenderAddon/` (`io_import_p3d`) loads that DLL through `ctypes`.

**Foundation** (`engine/Poseidon/Foundation/`) is the engine's own standard-library layer — custom Containers, Strings, Memory (mimalloc-backed), Threads, Math, Time, etc. — that the rest of the engine is written against instead of the STL. `PoseidonPCH.hpp` is the prelude precompiled-and-force-included into most targets; building with `POSEIDON_DISABLE_PCH=ON` removes the force-include to audit per-file include self-containment.

**Apps** (`apps/`, folder → CMake target): `cwr/Game` → `PoseidonGame`, `cwr/GameDemo` → `PoseidonGameDemo`, `cwr/Server` → `PoseidonServer` (all three link the `cwr/GameBase` static lib for shared startup). Tools under `tools/`: `Tools` → `PoseidonTools`, `Evaluator` → `PoseidonEvaluator`, `Studio` → `PoseidonStudio` (ImGui shell), and the Total Commander plugins `TcPbo` / `TcLister` (output names `pbo` / `poseidon`). `tools/BlenderAddon/` is a Python Blender addon, not a CMake target. Sample `tetris/Tetris` → `PoseidonTetris`. `fuzzers/Fuzzer` holds libFuzzer harnesses, added only when `POSEIDON_BUILD_FUZZERS` is on. GUI client targets link engine + GL/AL backends; tool targets link only the engine libs they need.

**Rust tooling** (one Cargo workspace, `arma_CWR/Cargo.toml`): `engine/Trident/` (the `tri` integration test runner / harness) and the `mserver/` crates: `Archive` (PBO pack/unpack), `CLI` (the `papa` binary: pack PBOs, publish mods), `Client` (protocol client SDK) and `MasterService` (the master-server service).

## Conventions that will surprise you

- **Legacy warning suppression is huge and intentional.** The root `CMakeLists.txt` disables 46 clang warning classes in one global `add_compile_options` block (verified to suppress ~11,600 warnings), and `engine/Poseidon/CMakeLists.txt` adds a few more (plus `-Wno-error`) scoped to the `Poseidon` target. Don't "fix" code just because a normally-warned pattern appears — many are deliberate (e.g. `memcpy`/`memmove` on bitwise-movable types via the `ClassIsMovableZeroed` pattern, intentionally incomplete `switch` in AI logic, defensive tautological compares).
- **clang-cl vs GNU clang differ on `__FILE__`.** On GNU-driver clang, `-fmacro-prefix-map` rewrites `__FILE__` relative to `arma_CWR/` (the CMake source dir, not the git root); on clang-cl it is left absolute on purpose (the flag is not forwarded via `/clang:`). No code currently parses `__FILE__` for paths: tests that need the repo root use `RepoRoot()` helpers built on the `TESTS_ROOT_DIR` compile definition (`tests/unit/engine/Poseidon/CMakeLists.txt`). Keep this asymmetry in mind when touching path/log/assert code.
- **PCH config is per-target PRIVATE**, not `REUSE_FROM`, because targets bake differing compile definitions into the PCH blob. See the `POSEIDON_PCH_TARGETS` list in the root `CMakeLists.txt`.
- The build adds targets in a specific order in the root `CMakeLists.txt`; new subdirectories must be registered there.
