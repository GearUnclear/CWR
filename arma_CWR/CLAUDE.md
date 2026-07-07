# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

This directory holds the engine and overhaul for **Uslu dur!** (**UD**) — a **total game overhaul** of *Arma: Cold War Assault* (originally *Operation Flashpoint*, 2001), not just an engine port. "Uslu dur!" is the public brand; the rename is branding-only, so the `cwr`/`CWR`/`Poseidon` codenames throughout the source are unchanged and stay as-is.

> **Repo layout note:** the git root is one level up at `D:\Arma_CWA`, not here. `arma_CWR/` is now a subdirectory alongside `site/` (the web portal) and `@LoBo/` (a mod test bed). See `D:\Arma_CWA\CLAUDE.md` for the repo-wide map; this file covers the engine + overhaul that live under `arma_CWR/`.

Two layers live here:

- **The upstream engine** (`engine/`, `apps/`, etc.), codename **Poseidon**. Modernized to C++20, built with CMake + Clang, cross-platform Windows x64 / Linux x64. This part is a **locked** Bohemia Interactive source release: PRs are not accepted upstream and there is no CI workflow checked into the repo (the sub-READMEs describe what CI *did* run). The code is GPL-3.0-or-later; game data (models, textures, sounds, missions) is **not** in this repo and ships separately under APL-SA.
- **The overhaul itself** ([`mod-plans/`](mod-plans/README.md), [`guerrilla-mode/`](guerrilla-mode/README.md)) — a from-the-ground-up rework of AI tactics/perception plus **Guerrilla Mode**, a new persistent open-world insurgency game mode with swappable islands and factions chosen at new-game. This is where active development happens. **Direction (2026-07-01): this is a total overhaul of the engine source, not a mod on a fixed binary — C++ engine changes are a first-class tool here, not a last resort, alongside SQF mission scripting.** Treat the mod-plans as living, AI-drafted design docs subject to correction, not settled hard specs.

## Working map (where to look / what to skip)

Most of this engine is 1,300+ C++ files, 160 of them over 1,000 lines. To avoid reading extraneous code:

**Active overhaul surface — start here:**
- `engine/Poseidon/Game/Guerrilla/` — `ZoneRegistry`, `GarrisonCache`, `AlertMachine` (+ their `*Commands.cpp` SQF bindings). The Phase-1.5 native subsystems.
- `engine/Poseidon/UI/Guerrilla/` — `GuerrillaModule`, `GuerrillaNewGame` (new-game island/faction selection).
- `engine/Poseidon/AI/` — the AI tactics/perception rework. See [`mod-plans/`](mod-plans/README.md) for which behavior maps to which files; treat those docs as intent, not a precise index.
- `voice-lines/` — TTS pipeline for custom in-game speech (WAVs via OpenRouter, played through `CfgSounds` + `say`/`sideRadio`; no engine changes). Key comes from the gitignored repo-root `.env`.

**Frozen upstream — read as reference, don't expect to edit:** `Network/`, `Graphics/`, and most of `World/` and `UI/`. This is a locked Bohemia source release (no upstream PRs). You'll read here to understand mechanisms, but the overhaul rarely changes them.

**Do NOT read or grep these — generated or non-source, pure context waste:**
- `thirdparty/glad/` — ~22k lines of generated OpenGL bindings.
- `@LoBo/` (mod test data, 2.2G binary), `build/`, and `ARMA Cold War Assault/AddOns/` (base game data).
- A repo-root `.ignore` already excludes these from `Grep`/`Glob`; the note here is for `Read`.

**Navigating big files efficiently:** grep for the symbol first, then `Read` a ±80-line window (`offset`/`limit`) rather than the whole 2,500–3,300-line file. For open-ended "where/how does X work" questions, delegate to an `Explore` subagent so the file dumps stay in *its* context and only the conclusion returns to yours.

## Toolchain prerequisites

Builds require, on PATH / in env:
- **Clang** — the only supported compiler (`clang-cl` frontend on Windows, GNU-driver clang on Linux). No MSVC, no GCC.
- **Ninja** — the generator used by every preset.
- **vcpkg** — manifest mode (`vcpkg.json`). `VCPKG_ROOT` env var must point at the vcpkg checkout; the base preset chainloads its toolchain.
- **ccache** — set as the compiler launcher by the base preset.

## Local environment (this Windows machine)

**Engine location:** `D:\Arma_CWA\arma_CWR` (moved here from `C:\dev\arma_CWR` on 2026-06-28 to sit next to the game data). The **git root is the parent** `D:\Arma_CWA` — run git and path-relative commands accordingly. A reminder shortcut `C:\dev\arma_CWR.lnk` points back here.

**Game data:** `D:\Arma_CWA\ARMA Cold War Assault` — the full installed game (AddOns, Campaigns, Worlds, `ColdWarAssault.exe`). Use this path for Trident's `OFPR_DATA_DIR` in `.trident.env`.

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

Configure-preset families (suffix = build type): `*-clang-dbg` (Debug), `*-clang-rwdi` (RelWithDebInfo), `*-clang-rel` (Release), for `win-x64` and `linux-x64`. Plus `linux-x64-steamrt4` (Steam Runtime), sanitizer presets `*-clang-san` / `linux-x64-clang-tsan`, and fuzzer presets `*-clang-fuzz` (turn on `POSEIDON_BUILD_FUZZERS`). The binary dir always mirrors the preset name under `build/`. Compiled apps are staged into `dist/<arch>-<platform>-<suffix>/`.

## Run

`run-game.ps1` (repo root) is the fast path to eyeball a change: it handles the
toolchain-on-PATH gotcha, does an incremental `PoseidonGame`-target build, and launches
the exe windowed against the game data dir.

```powershell
.\run-game.ps1                                      # build + launch to the main menu
.\run-game.ps1 -Mission 'Missions\Guerrilla.Demo'   # build + jump straight into a mission
.\run-game.ps1 -SkipBuild                           # launch the existing dist/ build as-is
```

Default `-DataDir` is `D:\Arma_CWA\ARMA Cold War Assault`. `-Mission` is resolved relative
to `-DataDir` and passed as `--test-mission` (omit it to boot to the main menu, where
GUERRILLA lets you pick island/faction interactively).

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
foreground of an AI session to "see" its output — use the log files.

**Always close what you open.** No human is at the keyboard to close game windows, and a
lingering instance poisons the next Trident connection ("connection error: eval failed").
Every automated command that spawns game instances (run-game.ps1, `tri test`) must
guarantee cleanup in the same command, e.g. append:
`powershell -NoProfile -Command "Get-Process PoseidonGame -ErrorAction SilentlyContinue | Stop-Process -Force"`
— and for run-game.ps1 launches, stop the specific PID once done reading logs. Run `tri`
with `--retries` (guerrilla_native_spawn's doMove assert is known-flaky) and check for
stray `PoseidonGame` processes before diagnosing connection failures.

## Tests

CTest drives both Catch2 unit tests and Trident integration tests:

```sh
ctest --test-dir build/win-x64-clang-rwdi --output-on-failure
ctest --test-dir build/win-x64-clang-rwdi -R "<test name>" --output-on-failure   # single test / regex
```

- **Unit** (`tests/unit/`, Catch2): suites `PoseidonFoundationTests`, `PoseidonCoreTests`, `PoseidonTests`, `PoseidonServerTests`, `PoseidonEvaluatorTests`, `PoseidonTetrisTests`. Catch2 tag filters like `[config]`, `[graphics]` partition `PoseidonTests`.
- **Integration** (`tests/integration/`, Trident): SQF-driven in-game scenarios. Needs game data. Build the runner first, then run:
  ```sh
  cargo build --manifest-path engine/Trident/Cargo.toml
  tri test -j6 --retries 2 tests/integration   # binary: engine/Trident/target/debug/tri[.exe]
  ```
  Copy `.trident.env.example` → `.trident.env`, set `OFPR_GAME_DIR` (build/dist dir with binaries) and `OFPR_DATA_DIR` (Demo game data; recommended layout `packages/Demo`, which is gitignored). Get free Demo data from Steam app 4819000.

  **On this machine** `.trident.env` points `OFPR_DATA_DIR` at the full CWA install (no Demo dataset), so the Demo-world tests (`guerrilla_capture_flip`, `guerrilla_spawn_domove`, `guerrilla_save_reload.seq`) cannot run here — the runnable Guerrilla set is the six `full_cwa` tests.

### Guerrilla integration-test preconditions (verified 2026-07-07)

- `ui/guerrilla_new_game_e2e` and `guerrilla_sinai_swap` need the mission templates **installed in the game dir**: `guerrilla-mode/install-missions.ps1 -IncludeWorld Sinai`. Without `-IncludeWorld Sinai` the installer's world gate **removes** Guerrilla.Sinai (Sinai's `.wrp` lives inside `@LoBo`'s pbo, invisible to the gate) and the e2e fails at `triSimUntil { alive player }` with a mount-failure message box behind it.
- The LoBo tests also need the one-time patched pbos: `tests/fixtures/mods-lobo/@lobofixup/gen-patched-pbos.ps1` (stock `@LoBo` pbos hard-abort under `--autotest` on malformed `tracerColor` floats).
- Run `tri` with `--retries` and low `-j`: `guerrilla_native_spawn` is known-flaky on its doMove displacement assert (recovers on retry), and parallel instances plus any manually closed window show up as `connection error: eval failed` cascades.

### Unit-test gotchas on Windows (verified 2026-06-28)

A clean build passes ~3536/3561 unit tests. The ~25 "failures" out of the box on this machine are **environment portability bugs in the tests, not engine regressions** — they reproduce on any fresh Windows/non-UTC checkout. Two causes, both fixed locally:

- **Hardcoded `/tmp/` paths (24 tests).** Ten test files (`test_paa_encoder`, `test_asset_preview`, `test_dds_converter`, `test_findfirst`, `test_gamePaths`, `test_crash_handler`, `test_mission_info`, `test_game_state_ext`, `test_dir_scanner`, `test_network_server`) write to literal `"/tmp/..."`. A native Windows exe resolves that to `\tmp\` **on the current drive**, so file writes (`WritePAA`, `saveToFile`, world-state writes) fail with no such directory. Fix: create `D:\tmp` (ctest runs from the `D:` drive). If you ever run the tests from another drive, create `\tmp` on that drive too.
- **`FormatTime` timezone assumption (1 test).** `AssetInfo::FormatTime` correctly renders asset mod-times in **local** time; the test in `test_asset_info.cpp` assumed UTC and asserted the string contained `"2020"` for `1577836800`, which renders as `2019-12-31 16:00:00` west of UTC. Fixed the test to assert the `YYYY-MM-DD HH:MM:SS` *format* instead (production code unchanged — local-time display is the desired behavior).

## Lint / format

These are custom CMake targets (not standalone scripts) — build them against a configured dir:

```sh
cmake --build build/win-x64-clang-rwdi --target Format      # clang-format check (--Werror)
cmake --build build/win-x64-clang-rwdi --target FormatFix   # clang-format in place
cmake --build build/win-x64-clang-rwdi --target Tidy        # clang-tidy
cmake --build build/win-x64-clang-rwdi --target Lint        # Format + Python (ruff, if uv present)
cmake --build build/win-x64-clang-rwdi --target FileSize    # warn >3000 lines, error >5000
```

Rust crates (`engine/Trident`, `mserver/*`) use the normal `cargo fmt` / `clippy` / `test` / `build` per their `Cargo.toml`.

## Architecture

**Engine (C++20 static libs, `engine/`)** — the stack apps link against:
- **Poseidon** (`engine/Poseidon/`, → `Poseidon.lib`) — the whole engine in one lib. Subsystems: `AI`, `Asset`, `Audio`, `Core`, `Dev`, `Foundation`, `Game`, `Graphics` (backend interface + factory), `IO` (streams, ParamFile, preproc), `Input`, `Network`, `Security`, `UI`, `World`, plus the SQF `Evaluator`.
- **PoseidonGL33** / **PoseidonOpenAL** — OpenGL 3.3 and OpenAL backends, linked only by client apps that need rendering/audio.
- **PoseidonFormats** — standalone C-API shared lib for P3D/PAA/PBO/RTM file formats; consumed by the Python Blender addon.

**Foundation** (`engine/Poseidon/Foundation/`) is the engine's own standard-library layer — custom Containers, Strings, Memory (mimalloc-backed), Threads, Math, Time, etc. — that the rest of the engine is written against instead of the STL. `PoseidonPCH.hpp` is the prelude precompiled-and-force-included into most targets; building with `POSEIDON_DISABLE_PCH=ON` removes the force-include to audit per-file include self-containment.

**Apps** (`apps/`): `cwr/Game` + `cwr/GameDemo` + `cwr/Server` (sharing `cwr/GameBase` startup), and tools under `tools/` (`Tools`/`PoseidonTools`, `Evaluator`, `Studio` ImGui shell, `TcPbo`, `TcLister`). Sample target `tetris/Tetris`, fuzz harnesses `fuzzers/Fuzzer`. GUI client targets link engine + GL/AL backends; tool targets link only the engine libs they need. Note the output name often differs from the target name (e.g. target `TcPbo` → `pbo`, `TcLister` → `poseidon`).

**Rust tooling**: `engine/Trident/` (the `tri` integration test runner / harness) and `mserver/` crates (`Archive`, `CLI`, `Client`, `MasterService` — public master-server service and CLI).

## Conventions that will surprise you

- **Legacy warning suppression is huge and intentional.** `CMakeLists.txt` disables ~50 clang warning classes globally (verified to suppress ~11,600 warnings). Don't "fix" code just because a normally-warned pattern appears — many are deliberate (e.g. `memcpy`/`memmove` on bitwise-movable types via the `ClassIsMovableZeroed` pattern, intentionally incomplete `switch` in AI logic, defensive tautological compares).
- **clang-cl vs GNU clang differ on `__FILE__`.** On GNU-driver clang, `-fmacro-prefix-map` rewrites `__FILE__` to repo-root-relative; on clang-cl it is left absolute on purpose (test helpers like `RepoPath()` walk an absolute `__FILE__` up to the repo root). Keep this asymmetry in mind when touching path/log/assert code.
- **PCH config is per-target PRIVATE**, not `REUSE_FROM`, because targets bake differing compile definitions into the PCH blob. See the `POSEIDON_PCH_TARGETS` list in the root `CMakeLists.txt`.
- The build adds targets in a specific order in the root `CMakeLists.txt`; new subdirectories must be registered there.
